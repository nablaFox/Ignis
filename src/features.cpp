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
