#pragma once

#include <vulkan/vulkan_core.h>
#include <string>
#include <vector>

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

bool checkExtensionsCompatibility(
	VkPhysicalDevice device,
	const std::vector<std::string>& requiredExtensions);

}  // namespace ignis
