#pragma once

#include "image.hpp"

namespace ignis {

enum class ColorFormat {
	RGBA8 = VK_FORMAT_R8G8B8A8_UNORM,
	RGBA16 = VK_FORMAT_R16G16B16A16_SFLOAT,
	HDR = VK_FORMAT_R32G32B32A32_SFLOAT,
};

class ColorImage : public Image {
public:
	struct CreateInfo {
		Device* device;
		ColorFormat format;
		VkExtent2D extent;
		VkImageUsageFlagBits usage;
		VkImageLayout optimalLayout;
		const void* initialPixels;
	};

	ColorImage(CreateInfo);

	// will use as the optimal layout COLOR_ATTACHMENT_OPTIMAL
	// and as the usage COLOR_ATTACHMENT_BIT
	struct DrawImageCreateInfo {
		Device* device;
		VkExtent2D extent;
		ColorFormat format;
	};

	static ColorImage* createDrawImage(DrawImageCreateInfo);
};

}  // namespace ignis
