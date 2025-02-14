#include <cstring>
#include <iostream>
#include "device.hpp"
#include "exceptions.hpp"
#include "features.hpp"

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

	if (func == nullptr)
		throw Exception("Failed to load vkCreateDebugUtilsMessengerEXT");

	THROW_VULKAN_ERROR(func(instance, &createInfo, nullptr, debugMessenger),
					   "Failed to allocate debug messenger");
}
#endif

static void createInstance(VkInstance* instance, std::string appName) {
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
	std::vector<const char*> enabledExtensions;

	if (checkValidationLayerSupport()) {
		enabledLayers.push_back("VK_LAYER_KHRONOS_validation");
		createInstanceInfo.enabledLayerCount = 1;
		createInstanceInfo.ppEnabledLayerNames = enabledLayers.data();

		enabledExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		createInstanceInfo.enabledExtensionCount = 1;
		createInstanceInfo.ppEnabledExtensionNames = enabledExtensions.data();
	}
#endif

	THROW_VULKAN_ERROR(vkCreateInstance(&createInstanceInfo, nullptr, instance),
					   "Failed to create instance");
}

static void getPhysicalDevice(VkInstance instance,
							  VkPhysicalDevice* device,
							  const std::vector<std::string> requiredExtensions) {
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	if (!deviceCount)
		throw Exception("Failed to find GPUs with Vulkan support");

	std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices.data());

	for (const auto& physicalDevice : physicalDevices) {
		bool featuresCompatible =
			RequiredFeatures::checkCompatibility(physicalDevice);

		if (!featuresCompatible)
			continue;

		bool extensionsCompatible =
			checkExtensionsCompatibility(physicalDevice, requiredExtensions);

		if (!extensionsCompatible)
			continue;

		*device = physicalDevice;
	}

	// TODO: show what are the incompatibilies
	if (*device == VK_NULL_HANDLE)
		throw Exception("Failed to find a suitable GPU");
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
								std::vector<VkQueue>* queues,
								VkDevice* device) {
	std::vector<float> priorities(graphicsQueuesCount, 1.0f);

	VkDeviceQueueCreateInfo queueCreateInfo{
		.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		.queueFamilyIndex = graphicsFamilyIndex,
		.queueCount = graphicsQueuesCount,
		.pQueuePriorities = priorities.data(),
	};

	RequiredFeatures features;
	auto& featuresChain = features.physicalDeviceFeatures2;

	VkDeviceCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.pNext = &featuresChain,
		.queueCreateInfoCount = 1,
		.pQueueCreateInfos = &queueCreateInfo,
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
		.physicalDevice = physicalDevice,
		.device = device,
		.instance = instance,
	};

	THROW_VULKAN_ERROR(vmaCreateAllocator(&allocatorInfo, allocator),
					   "Failed to create allocator");
}

Device::Device(CreateInfo createInfo) : m_shadersFolder(createInfo.shadersFolder) {
	createInstance(&m_instance, createInfo.appName);

#ifndef NDEBUG
	createDebugUtilsMessenger(m_instance, &m_debugMessenger);
#endif

	getPhysicalDevice(m_instance, &m_phyiscalDevice, createInfo.extensions);

	getGraphicsFamily(m_phyiscalDevice, &m_graphicsQueuesCount,
					  &m_graphicsFamilyIndex);

	createLogicalDevice(m_phyiscalDevice, m_graphicsQueuesCount,
						m_graphicsFamilyIndex, &m_queues, &m_device);

	createAllocator(m_device, m_phyiscalDevice, m_instance, &m_allocator);
}

Device::~Device() {
	vkDestroyDevice(m_device, nullptr);

#ifndef NDEBUG
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
		m_instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func)
		func(m_instance, m_debugMessenger, nullptr);
#endif

	vkDestroyInstance(m_instance, nullptr);
}
