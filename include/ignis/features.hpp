#pragma once

#include <vulkan/vulkan_core.h>

namespace ignis {

struct RequiredFeatures {
	VkPhysicalDeviceBufferDeviceAddressFeatures bufferDeviceAddress{};
	VkPhysicalDeviceDynamicRenderingFeatures dynamicRendering{};
	VkPhysicalDeviceSynchronization2FeaturesKHR syncrhonization2{};
	VkPhysicalDeviceDescriptorIndexingFeatures descriptorIndexing{};

	VkPhysicalDeviceFeatures2 physicalDeviceFeatures2{};

	RequiredFeatures();

	static bool checkCompatibility(VkPhysicalDevice device);
};

}  // namespace ignis
