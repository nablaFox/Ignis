#include "features.hpp"
#include <cstring>

using namespace ignis;

FeaturesChain::FeaturesChain() {
	bufferDeviceAddress = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES,
		.pNext = nullptr,
	};

	dynamicRendering = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES,
		.pNext = &bufferDeviceAddress,
	};

	syncrhonization2 = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES_KHR,
		.pNext = &dynamicRendering,
	};

	descriptorIndexing = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES,
		.pNext = &syncrhonization2,
	};

	physicalDeviceFeatures = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
		.pNext = &descriptorIndexing,
	};
}

Features::Features(std::vector<const char*> requiredFeatures,
				   std::vector<const char*> optionalFeatures)
	: m_requiredFeatures(std::move(requiredFeatures)) {
	for (const char* feature : optionalFeatures) {
		m_requiredFeatures.push_back(feature);
	}

	for (const char* feature : m_requiredFeatures) {
		if (strcmp(feature, "BufferDeviceAddress") == 0) {
			chain.bufferDeviceAddress.bufferDeviceAddress = VK_TRUE;
		}

		if (strcmp(feature, "DynamicRendering") == 0) {
			chain.dynamicRendering.dynamicRendering = VK_TRUE;
		}

		if (strcmp(feature, "Synchronization2") == 0) {
			chain.syncrhonization2.synchronization2 = VK_TRUE;
		}

		if (strcmp(feature, "DescriptorIndexing") == 0) {
			auto& descriptorIndexing = chain.descriptorIndexing;

			descriptorIndexing.descriptorBindingUniformBufferUpdateAfterBind =
				VK_TRUE;

			descriptorIndexing.descriptorBindingSampledImageUpdateAfterBind =
				VK_TRUE;

			descriptorIndexing.descriptorBindingStorageBufferUpdateAfterBind =
				VK_TRUE;

			descriptorIndexing.descriptorBindingPartiallyBound = VK_TRUE;

			descriptorIndexing.runtimeDescriptorArray = VK_TRUE;
		}

		auto& features = chain.physicalDeviceFeatures.features;

		if (strcmp(feature, "SampleRateShading") == 0) {
			features.sampleRateShading = VK_TRUE;
		}

		if (strcmp(feature, "FillModeNonSolid") == 0) {
			features.fillModeNonSolid = VK_TRUE;
		}
	};
}

bool Features::checkCompatibility(VkPhysicalDevice device) {
	for (const char* feature : m_requiredFeatures) {
		if (!isFeatureEnabled(feature, device)) {
			return false;
		}
	}

	return true;
}

bool Features::isFeatureEnabled(const char* feature, VkPhysicalDevice device) {
	FeaturesChain chain{};

	vkGetPhysicalDeviceFeatures2(device, &chain.physicalDeviceFeatures);

	if (strcmp(feature, "SampleRateShading") == 0) {
		return chain.physicalDeviceFeatures.features.sampleRateShading == VK_TRUE;
	}

	if (strcmp(feature, "FillModeNonSolid") == 0) {
		return chain.physicalDeviceFeatures.features.fillModeNonSolid == VK_TRUE;
	}

	if (strcmp(feature, "BufferDeviceAddress") == 0) {
		return chain.bufferDeviceAddress.bufferDeviceAddress == VK_TRUE;
	}

	if (strcmp(feature, "DynamicRendering") == 0) {
		return chain.dynamicRendering.dynamicRendering == VK_TRUE;
	}

	if (strcmp(feature, "Synchronization2") == 0) {
		return chain.syncrhonization2.synchronization2 == VK_TRUE;
	}

	if (strcmp(feature, "DescriptorIndexing") == 0) {
		return chain.descriptorIndexing
					   .descriptorBindingUniformBufferUpdateAfterBind == VK_TRUE &&
			   chain.descriptorIndexing
					   .descriptorBindingSampledImageUpdateAfterBind == VK_TRUE &&
			   chain.descriptorIndexing
					   .descriptorBindingStorageBufferUpdateAfterBind == VK_TRUE &&
			   chain.descriptorIndexing.descriptorBindingPartiallyBound == VK_TRUE &&
			   chain.descriptorIndexing.runtimeDescriptorArray == VK_TRUE;
	}

	return false;
}
