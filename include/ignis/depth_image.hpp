#pragma once

#include "image.hpp"

namespace ignis {

enum class DepthFormat {
	D16_UNORM = VK_FORMAT_D16_UNORM,
	D24_UNORM_S8_UINT = VK_FORMAT_D24_UNORM_S8_UINT,
	D32_SFLOAT = VK_FORMAT_D32_SFLOAT
};

class DepthImage : public Image {
public:
	struct CreateInfo {
		const Device* device;
		DepthFormat format;
		VkExtent2D extent;
		VkImageUsageFlags usage;
		VkImageLayout optimalLayout;
		const void* initialPixels;
	};

	DepthImage(CreateInfo);

	// - optimal layout DEPTH_STENCIL_ATTACHMENT_OPTIMAL
	// - usage DEPTH_STENCIL_ATTACHMENT_BIT
	// - format DEPTH_STENCIL
	static DepthImage* createDepthStencilImage(const Device* device,
											   VkExtent2D extent);
};

}  // namespace ignis
