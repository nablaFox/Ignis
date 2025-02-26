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
	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
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

static void allocateCommandPools(VkDevice device,
								 uint32_t graphicsQueuesCount,
								 uint32_t graphicsFamilyIndex,
								 std::vector<VkCommandPool>* commandPools) {
	for (uint32_t i = 0; i < graphicsQueuesCount; i++) {
		VkCommandPoolCreateInfo poolInfo{
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,  // PONDER
			.queueFamilyIndex = graphicsFamilyIndex,
		};

		VkCommandPool commandPool = nullptr;
		THROW_VULKAN_ERROR(
			vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool),
			"Failed to create command pool");

		commandPools->push_back(commandPool);
	}
}

Device::Device(CreateInfo createInfo) : m_shadersFolder(createInfo.shadersFolder) {
	createInstance(createInfo.appName, createInfo.instanceExtensions, &m_instance);

#ifndef NDEBUG
	createDebugUtilsMessenger(m_instance, &m_debugMessenger);
#endif

	Features features(createInfo.requiredFeatures);

	::getPhysicalDevice(m_instance, createInfo.extensions, features,
						&m_phyiscalDevice, &m_physicalDeviceProperties);

	getGraphicsFamily(m_phyiscalDevice, &m_graphicsQueuesCount,
					  &m_graphicsFamilyIndex);

	createLogicalDevice(m_phyiscalDevice, m_graphicsQueuesCount,
						m_graphicsFamilyIndex, createInfo.extensions,
						features.physicalDeviceFeatures2, &m_queues, &m_device);

	createAllocator(m_device, m_phyiscalDevice, m_instance, &m_allocator);

	allocateCommandPools(m_device, m_graphicsQueuesCount, m_graphicsFamilyIndex,
						 &m_commandPools);

	m_bindlessResources.initialize(m_device);
}

void Device::submitCommands(std::vector<SubmitCmdInfo> submits,
							const Fence& fence) const {
	uint32_t queueIndex = submits[0].command->getQueueIndex();

#ifndef NDEBUG
	for (const auto& submit : submits) {
		if (submit.command->getQueueIndex() != queueIndex) {
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
			.commandBuffer = submit.command->getHandle(),
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

	vkQueueSubmit2(m_queues[queueIndex], static_cast<uint32_t>(submitInfos.size()),
				   submitInfos.data(), fence.getHandle());
}

VkCommandPool Device::getCommandPool(uint32_t index) const {
	THROW_ERROR(index >= m_commandPools.size(), "Invalid command pool index");
	return m_commandPools[index];
}

VkQueue Device::getQueue(uint32_t index) const {
	THROW_ERROR(index >= m_queues.size(), "Invalid queue index");
	return m_queues[index];
}

void Device::registerUBO(const Buffer& buffer, uint32_t index) {
	assert((buffer.getUsage() & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT) != 0 &&
		   "Buffer is not a UBO");

	VkDescriptorBufferInfo bufferInfo{
		.buffer = buffer.getHandle(),
		.offset = 0,
		.range = buffer.getSize(),
	};

	VkWriteDescriptorSet writeDescriptorSet{
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = m_bindlessResources.getDescriptorSet(),
		.dstBinding = IGNIS_UNIFORM_BUFFER_BINDING,
		.dstArrayElement = index,
		.descriptorCount = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		.pBufferInfo = &bufferInfo,
	};

	vkUpdateDescriptorSets(m_device, 1, &writeDescriptorSet, 0, nullptr);
}

void Device::registerSSBO(const Buffer& buffer, uint32_t index) {
	assert((buffer.getUsage() & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT) != 0 &&
		   "Buffer is not a SSBO");

	VkDescriptorBufferInfo bufferInfo{
		.buffer = buffer.getHandle(),
		.offset = 0,
		.range = buffer.getSize(),
	};

	VkWriteDescriptorSet writeDescriptorSet{
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = m_bindlessResources.getDescriptorSet(),
		.dstBinding = IGNIS_STORAGE_BUFFER_BINDING,
		.dstArrayElement = index,
		.descriptorCount = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		.pBufferInfo = &bufferInfo,
	};

	vkUpdateDescriptorSets(m_device, 1, &writeDescriptorSet, 0, nullptr);
}

void Device::registerSampledImage(const Image& image,
								  const Sampler& sampler,
								  uint32_t index) {
	VkDescriptorImageInfo imageInfo{
		.sampler = sampler.getHandle(),
		.imageView = image.getViewHandle(),
		.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
	};

	VkWriteDescriptorSet writeDescriptorSet{
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = m_bindlessResources.getDescriptorSet(),
		.dstBinding = IGNIS_IMAGE_SAMPLER_BINDING,
		.dstArrayElement = index,
		.descriptorCount = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		.pImageInfo = &imageInfo,
	};

	vkUpdateDescriptorSets(m_device, 1, &writeDescriptorSet, 0, nullptr);
}

std::string Device::getFullShaderPath(std::string shaderPath) const {
	if (shaderPath[0] == '/')
		return shaderPath;

	return m_shadersFolder + "/" + shaderPath;
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

Device::~Device() {
	vkDeviceWaitIdle(m_device);

	m_bindlessResources.cleanup(m_device);

	for (auto queue : m_queues)
		vkQueueWaitIdle(queue);

	for (auto commandPool : m_commandPools)
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
