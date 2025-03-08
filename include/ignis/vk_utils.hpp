#pragma once

#include <vulkan/vulkan_core.h>
#include <vector>

namespace ignis {

bool checkExtensionsCompatibility(
	VkPhysicalDevice device,
	const std::vector<const char*>& requiredExtensions);

struct TransitionInfo {
	VkAccessFlags srcAccessMask;
	VkAccessFlags dstAccessMask;
	VkPipelineStageFlags srcStage;
	VkPipelineStageFlags dstStage;
};

TransitionInfo getTransitionInfo(VkImageLayout oldLayout, VkImageLayout newLayout);

VkDeviceSize getPixelSize(VkFormat);

bool isColorFormat(VkFormat);

bool isDepthFormat(VkFormat);

}  // namespace ignis
