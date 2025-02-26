#include "features.hpp"
#include <cstring>

using namespace ignis;

Features::Features(std::vector<const char*> additionalFeatures)
	: m_additionalFeatures(std::move(additionalFeatures)) {
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

	// additional user requested features
	auto& features = physicalDeviceFeatures2.features;

	for (const auto& feature : m_additionalFeatures) {
		if (strcmp(feature, "sampleRateShading") == 0) {
			features.sampleRateShading = VK_TRUE;
		}

		if (strcmp(feature, "fillModeNonSolid") == 0) {
			features.fillModeNonSolid = VK_TRUE;
		}
	}
}

bool Features::checkCompatibility(VkPhysicalDevice device) {
	vkGetPhysicalDeviceFeatures2(device, &physicalDeviceFeatures2);

	if (bufferDeviceAddress.bufferDeviceAddress != VK_TRUE) {
		return false;
	}

	if (dynamicRendering.dynamicRendering != VK_TRUE) {
		return false;
	}

	if (syncrhonization2.synchronization2 != VK_TRUE) {
		return false;
	}

	if (descriptorIndexing.descriptorBindingUniformBufferUpdateAfterBind !=
			VK_TRUE ||
		descriptorIndexing.descriptorBindingSampledImageUpdateAfterBind != VK_TRUE ||
		descriptorIndexing.descriptorBindingStorageBufferUpdateAfterBind !=
			VK_TRUE ||
		descriptorIndexing.descriptorBindingPartiallyBound != VK_TRUE ||
		descriptorIndexing.runtimeDescriptorArray != VK_TRUE) {
		return false;
	}

	for (const auto& feature : m_additionalFeatures) {
		if (strcmp(feature, "sampleRateShading") == 0) {
			if (physicalDeviceFeatures2.features.sampleRateShading != VK_TRUE) {
				return false;
			}
		}

		if (strcmp(feature, "fillModeNonSolid") == 0) {
			if (physicalDeviceFeatures2.features.fillModeNonSolid != VK_TRUE) {
				return false;
			}
		}
	}

	return true;
}
