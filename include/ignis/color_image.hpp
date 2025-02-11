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
	ColorImage(Device&,
			   VkExtent2D,
			   ColorFormat,
			   VkImageUsageFlagBits,

			   // will try to infer the best layout if not specified
			   VkImageLayout optimalLayout = VK_IMAGE_LAYOUT_GENERAL,

			   std::vector<void*> initialPixels = {});

	// will use as the optimal layout COLOR_ATTACHMENT_OPTIMAL
	// and as the usage COLOR_ATTACHMENT_BIT
	static ColorImage createDrawImage(Device&, VkExtent2D, ColorFormat);
};

}  // namespace ignis
