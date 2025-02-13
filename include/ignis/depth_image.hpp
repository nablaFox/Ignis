#pragma once

#include "image.hpp"

namespace ignis {

enum class DepthFormat {
	LOW = VK_FORMAT_D16_UNORM,
	DEPTH_STENCIL = VK_FORMAT_D24_UNORM_S8_UINT,
	HIGH = VK_FORMAT_D32_SFLOAT
};

class DepthImage : public Image {
public:
	struct CreateInfo {
		Device* device;
		DepthFormat format;
		VkExtent2D extent;
		VkImageUsageFlagBits usage;
		VkImageLayout optimalLayout;
		const void* initialPixels;
	};

	DepthImage(CreateInfo);

	// will use as the optimal layout DEPTH_STENCIL_ATTACHMENT_OPTIMAL
	// and as the usage DEPTH_STENCIL_ATTACHMENT_BIT
	// and the format DEPTH_STENCIL
	struct DepthStencilCreateInfo {
		Device* device;
		VkExtent2D extent;
	};

	static DepthImage* createDepthStencilImage(DepthStencilCreateInfo);
};

}  // namespace ignis
