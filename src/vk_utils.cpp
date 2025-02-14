#include "vk_utils.hpp"

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

ignis::TransitionInfo ignis::getTransitionInfo(VkImageLayout oldLayout,
											   VkImageLayout newLayout) {
	TransitionInfo info{};
	return info;
}
