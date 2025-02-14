#pragma once

#include <vulkan/vulkan_core.h>

namespace ignis {

struct RequiredFeatures {
	VkPhysicalDeviceBufferDeviceAddressFeatures
		physicalDeviceBufferDeviceAddressFeatures{};
	VkPhysicalDeviceDynamicRenderingFeatures
		physicalDeviceDynamicRenderingFeatures{};
	VkPhysicalDeviceFeatures2 physicalDeviceFeatures2{};

	RequiredFeatures();

	static bool checkCompatibility(VkPhysicalDevice device);
};

}  // namespace ignis
