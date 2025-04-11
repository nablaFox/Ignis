#pragma once

#include "ignis/device.hpp"

namespace ignis {

struct FeaturesChain {
	FeaturesChain();

	VkPhysicalDeviceBufferDeviceAddressFeatures bufferDeviceAddress{};
	VkPhysicalDeviceDynamicRenderingFeatures dynamicRendering{};
	VkPhysicalDeviceSynchronization2FeaturesKHR syncrhonization2{};
	VkPhysicalDeviceDescriptorIndexingFeatures descriptorIndexing{};

	VkPhysicalDeviceFeatures2 physicalDeviceFeatures{};
};

class Device::Features {
public:
	Features(std::vector<const char*> requiredFeatures,
			 std::vector<const char*> optionalFeatures);

	static bool isFeatureEnabled(const char* feature, VkPhysicalDevice);

	bool checkCompatibility(VkPhysicalDevice device) const;

	VkPhysicalDeviceFeatures2 getFeatures() const {
		return chain.physicalDeviceFeatures;
	}

	void pickPhysicalDevice(const VkInstance,
							const std::vector<const char*>& requiredExtensions,
							VkPhysicalDevice*,
							VkPhysicalDeviceProperties*) const;

private:
	FeaturesChain chain;
	std::vector<const char*> m_requiredFeatures;
};

}  // namespace ignis
