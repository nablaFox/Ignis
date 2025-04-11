#include <cstring>
#include "features.hpp"
#include "exceptions.hpp"

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

Device::Features::Features(std::vector<const char*> requiredFeatures,
						   std::vector<const char*> optionalFeatures)
	: m_requiredFeatures(requiredFeatures) {
	for (const char* feature : optionalFeatures) {
		requiredFeatures.push_back(feature);
	}

	for (const char* feature : requiredFeatures) {
		if (strcmp(feature, "BufferDeviceAddress") == 0) {
			chain.bufferDeviceAddress.bufferDeviceAddress = VK_TRUE;
		}

		if (strcmp(feature, "DynamicRendering") == 0) {
			chain.dynamicRendering.dynamicRendering = VK_TRUE;
		}

		if (strcmp(feature, "Synchronization2") == 0) {
			chain.syncrhonization2.synchronization2 = VK_TRUE;
		}

		auto& descriptorIndexing = chain.descriptorIndexing;

		if (strcmp(feature, "DescriptorBindingUniformBufferUpdateAfterBind") == 0) {
			descriptorIndexing.descriptorBindingUniformBufferUpdateAfterBind =
				VK_TRUE;
		}

		if (strcmp(feature, "DescriptorBindingSampledImageUpdateAfterBind") == 0) {
			descriptorIndexing.descriptorBindingSampledImageUpdateAfterBind =
				VK_TRUE;
		}

		if (strcmp(feature, "DescriptorBindingStorageBufferUpdateAfterBind") == 0) {
			descriptorIndexing.descriptorBindingStorageBufferUpdateAfterBind =
				VK_TRUE;
		}

		if (strcmp(feature, "DescriptorBindingPartiallyBound") == 0) {
			descriptorIndexing.descriptorBindingPartiallyBound = VK_TRUE;
		}

		if (strcmp(feature, "RuntimeDescriptorArray") == 0) {
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

bool Device::Features::checkCompatibility(VkPhysicalDevice device) const {
	for (const char* feature : m_requiredFeatures) {
		if (!isFeatureEnabled(feature, device)) {
			return false;
		}
	}

	return true;
}

bool Device::Features::isFeatureEnabled(const char* feature,
										VkPhysicalDevice device) {
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

	if (strcmp(feature, "DescriptorBindingUniformBufferUpdateAfterBind") == 0) {
		return chain.descriptorIndexing
				   .descriptorBindingUniformBufferUpdateAfterBind == VK_TRUE;
	}

	if (strcmp(feature, "DescriptorBindingSampledImageUpdateAfterBind") == 0) {
		return chain.descriptorIndexing
				   .descriptorBindingSampledImageUpdateAfterBind == VK_TRUE;
	}

	if (strcmp(feature, "DescriptorBindingStorageBufferUpdateAfterBind") == 0) {
		return chain.descriptorIndexing
				   .descriptorBindingStorageBufferUpdateAfterBind == VK_TRUE;
	}

	if (strcmp(feature, "DescriptorBindingPartiallyBound") == 0) {
		return chain.descriptorIndexing.descriptorBindingPartiallyBound == VK_TRUE;
	}

	if (strcmp(feature, "RuntimeDescriptorArray") == 0) {
		return chain.descriptorIndexing.runtimeDescriptorArray == VK_TRUE;
	}

	return false;
}

static bool checkExtensionsCompatibility(
	VkPhysicalDevice device,
	const std::vector<const char*>& requiredExtensions) {
	uint32_t extensionCount = 0;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
										 availableExtensions.data());

	for (const auto& requiredExt : requiredExtensions) {
		bool found = false;
		for (const auto& availableExt : availableExtensions) {
			if (strcmp(requiredExt, availableExt.extensionName) == 0) {
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

void Device::Features::pickPhysicalDevice(
	const VkInstance instance,
	const std::vector<const char*>& requiredExtensions,
	VkPhysicalDevice* device,
	VkPhysicalDeviceProperties* properties) const {
	uint32_t deviceCount{0};
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	THROW_ERROR(!deviceCount, "Failed to find a GPU with Vulkan support");

	std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices.data());

	for (const auto& physicalDevice : physicalDevices) {
		if (!checkCompatibility(physicalDevice))
			continue;

		if (!checkExtensionsCompatibility(physicalDevice, requiredExtensions))
			continue;

		*device = physicalDevice;
	}

	// TODO: show what are the incompatibilies
	THROW_ERROR(*device == VK_NULL_HANDLE, "Failed to find a suitable GPU");

	vkGetPhysicalDeviceProperties(*device, properties);
}
