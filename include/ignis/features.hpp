#pragma once

#include <vulkan/vulkan_core.h>
#include <vector>

namespace ignis {

struct FeaturesChain {
	FeaturesChain();

	VkPhysicalDeviceBufferDeviceAddressFeatures bufferDeviceAddress{};
	VkPhysicalDeviceDynamicRenderingFeatures dynamicRendering{};
	VkPhysicalDeviceSynchronization2FeaturesKHR syncrhonization2{};
	VkPhysicalDeviceDescriptorIndexingFeatures descriptorIndexing{};

	VkPhysicalDeviceFeatures2 physicalDeviceFeatures{};
};

class Features {
public:
	Features(std::vector<const char*> requiredFeatures,
			 std::vector<const char*> optionalFeatures);

	bool checkCompatibility(VkPhysicalDevice device);

	static bool isFeatureEnabled(const char* feature, VkPhysicalDevice);

	VkPhysicalDeviceFeatures2 getFeatures() const {
		return chain.physicalDeviceFeatures;
	}

private:
	FeaturesChain chain;
	std::vector<const char*> m_requiredFeatures;
};

}  // namespace ignis
