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

	descriptorIndexing = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES,
		.pNext = &syncrhonization2,
		.descriptorBindingUniformBufferUpdateAfterBind = VK_TRUE,
		.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE,
		.descriptorBindingStorageBufferUpdateAfterBind = VK_TRUE,
		.descriptorBindingPartiallyBound = VK_TRUE,
		.runtimeDescriptorArray = VK_TRUE,
	};

	physicalDeviceFeatures2 = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
		.pNext = &descriptorIndexing,
	};
}

bool RequiredFeatures::checkCompatibility(VkPhysicalDevice device) {
	RequiredFeatures features;
	vkGetPhysicalDeviceFeatures2(device, &features.physicalDeviceFeatures2);

	bool hasBufferDeviceAddress =
		features.bufferDeviceAddress.bufferDeviceAddress == VK_TRUE;

	bool hasDynamicRendering = features.dynamicRendering.dynamicRendering == VK_TRUE;

	bool hasSynchronization2 = features.syncrhonization2.synchronization2 == VK_TRUE;

	bool hasDescriptorIndexing =
		features.descriptorIndexing.descriptorBindingUniformBufferUpdateAfterBind ==
			VK_TRUE &&
		features.descriptorIndexing.descriptorBindingSampledImageUpdateAfterBind ==
			VK_TRUE &&
		features.descriptorIndexing.descriptorBindingStorageBufferUpdateAfterBind ==
			VK_TRUE &&
		features.descriptorIndexing.descriptorBindingPartiallyBound == VK_TRUE &&
		features.descriptorIndexing.runtimeDescriptorArray == VK_TRUE;

	return hasBufferDeviceAddress && hasDynamicRendering && hasSynchronization2 &&
		   hasDescriptorIndexing;
}
