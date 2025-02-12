#include "features.hpp"

using namespace ignis;

RequiredFeatures::RequiredFeatures() {
	physicalDeviceBufferDeviceAddressFeatures.sType =
		VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
	physicalDeviceBufferDeviceAddressFeatures.pNext =
		&physicalDeviceDynamicRenderingFeatures;

	physicalDeviceDynamicRenderingFeatures.sType =
		VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
	physicalDeviceDynamicRenderingFeatures.pNext = nullptr;

	physicalDeviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	physicalDeviceFeatures2.pNext = &physicalDeviceBufferDeviceAddressFeatures;

	// enable the features
	physicalDeviceBufferDeviceAddressFeatures.bufferDeviceAddress = VK_TRUE;
	physicalDeviceDynamicRenderingFeatures.dynamicRendering = VK_TRUE;
}

bool RequiredFeatures::checkCompatibility(VkPhysicalDevice device) {
	RequiredFeatures features;
	vkGetPhysicalDeviceFeatures2(device, &features.physicalDeviceFeatures2);

	bool hasBufferDeviceAddress =
		features.physicalDeviceBufferDeviceAddressFeatures.bufferDeviceAddress ==
		VK_TRUE;
	bool hasDynamicRendering =
		features.physicalDeviceDynamicRenderingFeatures.dynamicRendering == VK_TRUE;

	return hasBufferDeviceAddress && hasDynamicRendering;
}

bool ignis::checkExtensionsCompatibility(
	VkPhysicalDevice device,
	const std::vector<std::string>& requiredExtensions) {
	uint32_t extensionCount = 0;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
										 availableExtensions.data());

	for (const auto& requiredExt : requiredExtensions) {
		bool found = false;
		for (const auto& availableExt : availableExtensions) {
			if (requiredExt == availableExt.extensionName) {
				found = true;
				break;
			}
		}

		if (!found) {
			return false;
		}
	}

	return true;
}
