#pragma once

#include <vulkan/vulkan_core.h>
#include <string>
#include <vector>

namespace ignis {

bool checkExtensionsCompatibility(
	VkPhysicalDevice device,
	const std::vector<std::string>& requiredExtensions);

struct TransitionInfo {
	VkAccessFlags srcAccessMask;
	VkAccessFlags dstAccessMask;
	VkPipelineStageFlags srcStage;
	VkPipelineStageFlags dstStage;
};

TransitionInfo getTransitionInfo(VkImageLayout oldLayout, VkImageLayout newLayout);

VkDeviceSize getPixelSize(VkFormat);

}  // namespace ignis
