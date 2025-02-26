#pragma once

#include <vulkan/vulkan_core.h>
#include <vector>

namespace ignis {

struct Features {
	// ignis required features
	VkPhysicalDeviceBufferDeviceAddressFeatures bufferDeviceAddress{};
	VkPhysicalDeviceDynamicRenderingFeatures dynamicRendering{};
	VkPhysicalDeviceSynchronization2FeaturesKHR syncrhonization2{};
	VkPhysicalDeviceDescriptorIndexingFeatures descriptorIndexing{};

	Features(std::vector<const char*> additionalFeatures = {});

	VkPhysicalDeviceFeatures2 physicalDeviceFeatures2{};

	bool checkCompatibility(VkPhysicalDevice device);

	std::vector<const char*> m_additionalFeatures;
};

}  // namespace ignis
