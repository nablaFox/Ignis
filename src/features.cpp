#include "features.hpp"

using namespace ignis;

RequiredFeatures::RequiredFeatures() {
	bufferDeviceAddress = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES,
		.pNext = nullptr,
		.bufferDeviceAddress = VK_TRUE,
	};

	dynamicRendering = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES,
		.pNext = &bufferDeviceAddress,
		.dynamicRendering = VK_TRUE,
	};

	syncrhonization2 = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES_KHR,
		.pNext = &dynamicRendering,
		.synchronization2 = VK_TRUE,
	};

	physicalDeviceFeatures2 = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
		.pNext = &syncrhonization2,
	};
}

bool RequiredFeatures::checkCompatibility(VkPhysicalDevice device) {
	RequiredFeatures features;
	vkGetPhysicalDeviceFeatures2(device, &features.physicalDeviceFeatures2);

	bool hasBufferDeviceAddress =
		features.bufferDeviceAddress.bufferDeviceAddress == VK_TRUE;

	bool hasDynamicRendering = features.dynamicRendering.dynamicRendering == VK_TRUE;

	bool hasSynchronization2 = features.syncrhonization2.synchronization2 == VK_TRUE;

	return hasBufferDeviceAddress && hasDynamicRendering && hasSynchronization2;
}
