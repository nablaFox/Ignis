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

VkDeviceSize ignis::getPixelSize(VkFormat format) {
	switch (format) {
		case VK_FORMAT_D16_UNORM:
			return 2;
		case VK_FORMAT_D24_UNORM_S8_UINT:
			return 4;
		case VK_FORMAT_D32_SFLOAT:
			return 4;
		case VK_FORMAT_R8G8B8A8_UNORM:
			return 4;
		case VK_FORMAT_R16G16B16A16_SFLOAT:
			return 8;
		case VK_FORMAT_R32G32B32A32_SFLOAT:
			return 16;
		default:
			return 0;
	}
}
