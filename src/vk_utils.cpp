#include <cstring>
#include "vk_utils.hpp"
#include "exceptions.hpp"

ignis::TransitionInfo ignis::getTransitionInfo(VkImageLayout oldLayout,
											   VkImageLayout newLayout) {
	TransitionInfo info{};

	// UNDEFINED -> ANY
	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED) {
		info.srcAccessMask = 0;
		info.dstAccessMask = 0;
		info.srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		info.dstStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	}

	// PRESENT -> DST
	else if (oldLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR &&
			 newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		info.srcAccessMask = 0;
		info.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		info.srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		info.dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}

	// DST -> PRESENT
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
			 newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
		info.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		info.dstAccessMask = 0;
		info.srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		info.dstStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	}

	// SRC -> COLOR
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL &&
			 newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
		info.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		info.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		info.srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		info.dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	}

	// GENERIC -> ANY
	else if (oldLayout == VK_IMAGE_LAYOUT_GENERAL) {
		info.srcAccessMask = 0;
		info.dstAccessMask = 0;
		info.srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		info.dstStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	}

	// TRANSFER_DST_OPTIMAL -> COLOR_ATTACHMENT_OPTIMAL
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
			 newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
		info.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		info.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		info.srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		info.dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	}

	// COLOR_ATTACHMENT_OPTIMAL -> TRANSFER_DST_OPTIMAL
	else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL &&
			 newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		info.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		info.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		info.srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		info.dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}

	// COLOR_ATTACHMENT_OPTIMAL -> TRANSFER_SRC_OPTIMAL
	else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL &&
			 newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
		info.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		info.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		info.srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		info.dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	// TRANSFER_DST_OPTIMAL -> DEPTH_STENCIL_ATTACHMENT_OPTIMAL
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
			 newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		info.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		info.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		info.srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		info.dstStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}
	// DEPTH_STENCIL_ATTACHMENT_OPTIMAL -> TRANSFER_SRC_OPTIMAL
	else if (oldLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL &&
			 newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
		info.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		info.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		info.srcStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		info.dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	} else {
		const char* oldLayoutStr = string_VkImageLayout(oldLayout);
		const char* newLayoutStr = string_VkImageLayout(newLayout);

		THROW_ERROR(true, "Unsupported layout transition: " +
							  std::string(oldLayoutStr) + " -> " + newLayoutStr);
	}

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
		case VK_FORMAT_R16G16B16A16_UNORM:
			return 8;
		case VK_FORMAT_R32G32B32A32_SFLOAT:
			return 16;
		default:
			return 0;
	}
}

bool ignis::isColorFormat(VkFormat format) {
	switch (format) {
		case VK_FORMAT_R8G8B8A8_UNORM:
		case VK_FORMAT_R16G16B16A16_SFLOAT:
		case VK_FORMAT_R32G32B32A32_SFLOAT:
			return true;
		default:
			return false;
	}
}

bool ignis::isDepthFormat(VkFormat format) {
	switch (format) {
		case VK_FORMAT_D16_UNORM:
		case VK_FORMAT_D24_UNORM_S8_UINT:
		case VK_FORMAT_D32_SFLOAT:
			return true;
		default:
			return false;
	}
}
