#include <cstring>
#include <iostream>
#include "device.hpp"
#include "command.hpp"
#include "exceptions.hpp"
#include "features.hpp"
#include "fence.hpp"
#include "semaphore.hpp"
#include "buffer.hpp"
#include "image.hpp"
#include "sampler.hpp"
#include "vk_utils.hpp"
#include "gpu_resources.hpp"

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

using namespace ignis;

#ifndef NDEBUG
static bool checkValidationLayerSupport() {
	uint32_t layerCount = 0;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const auto& layer : availableLayers)
		if (strcmp(layer.layerName, "VK_LAYER_KHRONOS_validation") == 0)
			return true;

	return false;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			  VkDebugUtilsMessageTypeFlagsEXT messageType,
			  const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			  void* pUserData) {
	std::cerr << pCallbackData->pMessage << "\n" << std::endl;
	return VK_FALSE;
}

static void createDebugUtilsMessenger(VkInstance instance,
									  VkDebugUtilsMessengerEXT* debugMessenger) {
	VkDebugUtilsMessengerCreateInfoEXT createInfo{
		.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
		.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
						   VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
						   VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
		.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
					   VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
					   VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
		.pfnUserCallback = debugCallback,
	};

	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
		instance, "vkCreateDebugUtilsMessengerEXT");

	THROW_ERROR(func == nullptr, "Failed to load vkCreateDebugUtilsMessengerEXT");

	THROW_VULKAN_ERROR(func(instance, &createInfo, nullptr, debugMessenger),
					   "Failed to allocate debug messenger");
}
#endif

static void createInstance(std::string appName,
						   std::vector<const char*> extensions,
						   VkInstance* instance) {
	VkApplicationInfo appInfo{
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName = appName.c_str(),
		.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
		.pEngineName = "Ignis",
		.engineVersion = VK_MAKE_VERSION(1, 0, 0),
		.apiVersion = VK_API_VERSION_1_3,
	};

	VkInstanceCreateInfo createInstanceInfo{
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pApplicationInfo = &appInfo,
	};

#ifndef NDEBUG
	std::vector<const char*> enabledLayers;

	if (checkValidationLayerSupport()) {
		enabledLayers.push_back("VK_LAYER_KHRONOS_validation");
		createInstanceInfo.enabledLayerCount = 1;
		createInstanceInfo.ppEnabledLayerNames = enabledLayers.data();

		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}
#endif

	createInstanceInfo.enabledExtensionCount =
		static_cast<uint32_t>(extensions.size());
	createInstanceInfo.ppEnabledExtensionNames = extensions.data();

	THROW_VULKAN_ERROR(vkCreateInstance(&createInstanceInfo, nullptr, instance),
					   "Failed to create instance");
}

static void getPhysicalDevice(VkInstance instance,
							  const std::vector<const char*>& requiredExtensions,
							  Features& features,
							  VkPhysicalDevice* device,
							  VkPhysicalDeviceProperties* properties) {
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	THROW_ERROR(!deviceCount, "Failed to find a GPU with Vulkan support");

	std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices.data());

	for (const auto& physicalDevice : physicalDevices) {
		if (!features.checkCompatibility(physicalDevice))
			continue;

		if (!checkExtensionsCompatibility(physicalDevice, requiredExtensions))
			continue;

		*device = physicalDevice;
	}

	// TODO: show what are the incompatibilies
	THROW_ERROR(*device == VK_NULL_HANDLE, "Failed to find a suitable GPU");

	vkGetPhysicalDeviceProperties(*device, properties);
}

static void getGraphicsFamily(VkPhysicalDevice device,
							  uint32_t* graphicsQueuesCount,
							  uint32_t* graphicsFamilyIndex) {
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);

	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
											 queueFamilyProperties.data());

	for (uint32_t i = 0; i < queueFamilyCount; i++) {
		if (queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			*graphicsFamilyIndex = i;
			*graphicsQueuesCount = queueFamilyProperties[i].queueCount;
			return;
		}
	}
}

static void createLogicalDevice(VkPhysicalDevice physicalDevice,
								uint32_t graphicsQueuesCount,
								uint32_t graphicsFamilyIndex,
								const std::vector<const char*>& extensions,
								VkPhysicalDeviceFeatures2 features,
								std::vector<VkQueue>* queues,
								VkDevice* device) {
	std::vector<float> priorities(graphicsQueuesCount, 1.0f);

	VkDeviceQueueCreateInfo queueCreateInfo{
		.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		.queueFamilyIndex = graphicsFamilyIndex,
		.queueCount = graphicsQueuesCount,
		.pQueuePriorities = priorities.data(),
	};

	VkDeviceCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.pNext = &features,
		.queueCreateInfoCount = 1,
		.pQueueCreateInfos = &queueCreateInfo,
		.enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
		.ppEnabledExtensionNames = extensions.data(),
		.pEnabledFeatures = nullptr,
	};

	THROW_VULKAN_ERROR(vkCreateDevice(physicalDevice, &createInfo, nullptr, device),
					   "Failed to create logical device");

	for (uint32_t i = 0; i < graphicsQueuesCount; i++) {
		VkQueue queue = nullptr;
		vkGetDeviceQueue(*device, graphicsFamilyIndex, i, &queue);
		queues->push_back(queue);
	}
}

static void createAllocator(VkDevice device,
							VkPhysicalDevice physicalDevice,
							VkInstance instance,
							VmaAllocator* allocator) {
	VmaAllocatorCreateInfo allocatorInfo{
		.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
		.physicalDevice = physicalDevice,
		.device = device,
		.instance = instance,
	};

	THROW_VULKAN_ERROR(vmaCreateAllocator(&allocatorInfo, allocator),
					   "Failed to create allocator");
}

static void allocateCommandPools(
	VkDevice device,
	uint32_t graphicsFamilyIndex,
	const std::vector<VkQueue>& graphicsQueues,
	std::unordered_map<VkQueue, VkCommandPool>* commandPools) {
	for (const auto& queue : graphicsQueues) {
		VkCommandPoolCreateInfo poolInfo{
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
			.queueFamilyIndex = graphicsFamilyIndex,
		};

		VkCommandPool commandPool = nullptr;
		THROW_VULKAN_ERROR(
			vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool),
			"Failed to create command pool");

		commandPools->insert({queue, commandPool});
	}
}

Device::Device(const CreateInfo& createInfo)
	: m_shadersFolder(createInfo.shadersFolder) {
	createInstance(createInfo.appName, createInfo.instanceExtensions, &m_instance);

#ifndef NDEBUG
	createDebugUtilsMessenger(m_instance, &m_debugMessenger);
#endif

	std::vector<const char*> requiredFeatures = createInfo.requiredFeatures;

	for (const auto& reqFeature : IGNIS_REQ_FEATURES) {
		requiredFeatures.push_back(reqFeature);
	}

	auto m_features =
		std::make_unique<Features>(requiredFeatures, createInfo.optionalFeatures);

	::getPhysicalDevice(m_instance, createInfo.extensions, *m_features,
						&m_phyiscalDevice, &m_physicalDeviceProperties);

	getGraphicsFamily(m_phyiscalDevice, &m_graphicsQueuesCount,
					  &m_graphicsFamilyIndex);

	createLogicalDevice(m_phyiscalDevice, m_graphicsQueuesCount,
						m_graphicsFamilyIndex, createInfo.extensions,
						m_features->getFeatures(), &m_queues, &m_device);

	createAllocator(m_device, m_phyiscalDevice, m_instance, &m_allocator);

	allocateCommandPools(m_device, m_graphicsFamilyIndex, m_queues, &m_commandPools);

	BindlessResourcesCreateInfo bindlessResourcesCreateInfo{
		.device = m_device,
		.maxStorageBuffers =
			m_physicalDeviceProperties.limits.maxPerStageDescriptorStorageBuffers,
		.maxUniformBuffers =
			m_physicalDeviceProperties.limits.maxPerStageDescriptorUniformBuffers,
		.maxImageSamplers =
			m_physicalDeviceProperties.limits.maxPerStageDescriptorSampledImages,
		.storageBuffersBinding = IGNIS_STORAGE_BUFFER_BINDING,
		.uniformBuffersBinding = IGNIS_UNIFORM_BUFFER_BINDING,
		.imageSamplersBinding = IGNIS_IMAGE_SAMPLER_BINDING,
	};

	m_gpuResources = std::make_unique<GpuResources>(bindlessResourcesCreateInfo);
}

Device::~Device() {
	vkDeviceWaitIdle(m_device);

	m_gpuResources.reset();

	m_buffers.clear();

	m_images.clear();

	for (auto queue : m_queues)
		vkQueueWaitIdle(queue);

	for (const auto& [_, commandPool] : m_commandPools)
		vkDestroyCommandPool(m_device, commandPool, nullptr);

	vmaDestroyAllocator(m_allocator);

	vkDestroyDevice(m_device, nullptr);

#ifndef NDEBUG
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
		m_instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func)
		func(m_instance, m_debugMessenger, nullptr);
#endif

	vkDestroyInstance(m_instance, nullptr);
}

VkQueue Device::getQueue(uint32_t index) const {
	THROW_ERROR(index >= m_queues.size(), "Invalid queue index");
	return m_queues[index];
}

VkCommandPool Device::getCommandPool(VkQueue queue) const {
	auto it = m_commandPools.find(queue);

	THROW_ERROR(it == m_commandPools.end(), "Invalid queue");

	return it->second;
}

void Device::submitCommands(std::vector<SubmitCmdInfo> submits,
							const Fence* fence) const {
	VkQueue queue = submits[0].command.getQueue();

#ifndef NDEBUG
	for (const auto& submit : submits) {
		if (submit.command.getQueue() != queue) {
			std::cerr << "ignis::Device::submitCommands: "
						 "commands must be relative to the same queue"
					  << std::endl;
		}
	}
#endif

	struct SubmissionData {
		VkSubmitInfo2 submitInfo{};
		std::vector<VkSemaphoreSubmitInfo> waitInfos;
		std::vector<VkSemaphoreSubmitInfo> signalInfos;
		VkCommandBufferSubmitInfo commandInfo{};
	};

	std::vector<SubmissionData> submissionsData;
	submissionsData.reserve(submits.size());

	std::vector<VkSubmitInfo2> submitInfos;
	submitInfos.reserve(submits.size());

	for (const auto& submit : submits) {
		SubmissionData data;

		data.waitInfos.reserve(submit.waitSemaphores.size());
		for (const auto& waitSemaphore : submit.waitSemaphores) {
			data.waitInfos.push_back({
				.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
				.semaphore = waitSemaphore->getHandle(),
			});
		}

		data.signalInfos.reserve(submit.signalSemaphores.size());
		for (const auto& signalSemaphore : submit.signalSemaphores) {
			data.signalInfos.push_back({
				.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
				.semaphore = signalSemaphore->getHandle(),
			});
		}

		data.commandInfo = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
			.commandBuffer = submit.command.getHandle(),
		};

		data.submitInfo = {
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
			.waitSemaphoreInfoCount = static_cast<uint32_t>(data.waitInfos.size()),
			.pWaitSemaphoreInfos = data.waitInfos.data(),
			.commandBufferInfoCount = 1,
			.pCommandBufferInfos = &data.commandInfo,
			.signalSemaphoreInfoCount =
				static_cast<uint32_t>(data.signalInfos.size()),
			.pSignalSemaphoreInfos = data.signalInfos.data(),
		};

		submissionsData.push_back(std::move(data));
	}

	for (const auto& data : submissionsData) {
		submitInfos.push_back(data.submitInfo);
	}

	vkQueueSubmit2(queue, static_cast<uint32_t>(submitInfos.size()),
				   submitInfos.data(), fence ? fence->getHandle() : nullptr);
}

VkSampleCountFlagBits Device::getMaxSampleCount() const {
	VkSampleCountFlags counts =
		m_physicalDeviceProperties.limits.framebufferColorSampleCounts &
		m_physicalDeviceProperties.limits.framebufferDepthSampleCounts;

	if (counts & VK_SAMPLE_COUNT_64_BIT)
		return VK_SAMPLE_COUNT_64_BIT;
	if (counts & VK_SAMPLE_COUNT_32_BIT)
		return VK_SAMPLE_COUNT_32_BIT;
	if (counts & VK_SAMPLE_COUNT_16_BIT)
		return VK_SAMPLE_COUNT_16_BIT;
	if (counts & VK_SAMPLE_COUNT_8_BIT)
		return VK_SAMPLE_COUNT_8_BIT;
	if (counts & VK_SAMPLE_COUNT_4_BIT)
		return VK_SAMPLE_COUNT_4_BIT;
	if (counts & VK_SAMPLE_COUNT_2_BIT)
		return VK_SAMPLE_COUNT_2_BIT;

	return VK_SAMPLE_COUNT_1_BIT;
}

bool Device::isFeatureEnabled(const char* feature) const {
	return m_features->isFeatureEnabled(feature, m_phyiscalDevice);
}

Buffer Device::createStagingBuffer(VkDeviceSize size, const void* data) const {
	return Buffer::allocateStagingBuffer(m_allocator, size, data);
}

Image Device::createDrawAttachmentImage(const DrawImageCreateInfo& info) const {
	return Image::allocateDrawImage(m_device, m_allocator, info);
}

Image Device::createDepthAttachmentImage(const DepthImageCreateInfo& info) const {
	return Image::allocateDepthImage(m_device, m_allocator, info);
}

BufferId Device::createUBO(VkDeviceSize size, const void* data, BufferId givenId) {
	Buffer ubo = Buffer::allocateUBO(
		m_allocator,
		m_physicalDeviceProperties.limits.minUniformBufferOffsetAlignment, size,
		data);

	return m_gpuResources->registerBuffer(std::move(ubo), givenId);
}

BufferId Device::createSSBO(VkDeviceSize size, const void* data, BufferId givenId) {
	Buffer ssbo = Buffer::allocateSSBO(
		m_allocator,
		m_physicalDeviceProperties.limits.minStorageBufferOffsetAlignment, size,
		data);

	return m_gpuResources->registerBuffer(std::move(ssbo), givenId);
}

ImageId Device::createStorageImage(const ImageCreateInfo& info) {
	ImageCreateInfo actualInfo = info;
	actualInfo.usage |= VK_IMAGE_USAGE_STORAGE_BIT;

	Image image(m_device, m_allocator, actualInfo);

	return m_gpuResources->registerImage(std::move(image));
}

ImageId Device::createSampledImage(const ImageCreateInfo& info) {
	ImageCreateInfo actualInfo = info;
	actualInfo.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;

	Image image(m_device, m_allocator, actualInfo);

	return m_gpuResources->registerImage(std::move(image));
}

Buffer& Device::getBuffer(BufferId handle) const {
	auto it = m_buffers.find(handle);

	THROW_ERROR(it == m_buffers.end(), "Invalid buffer handle");

	return *it->second;
}

Image& Device::getImage(ImageId handle) const {
	auto it = m_images.find(handle);

	THROW_ERROR(it == m_images.end(), "Invalid image handle");

	return *it->second;
}

void Device::destroyBuffer(BufferId handle) {
	auto it = m_buffers.find(handle);

	THROW_ERROR(it == m_buffers.end(), "Invalid buffer handle");

	m_buffers.erase(it);
}

VkPipelineLayout Device::getPipelineLayout(uint32_t pushConstantSize) const {
	return m_gpuResources->getPipelinelayout(1 + (pushConstantSize / 4));
};

VkDescriptorSet Device::getDescriptorSet() const {
	return m_gpuResources->getDescriptorSet();
}
