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
	struct CreateInfo : Image::CreateInfo {
		ColorFormat format;
	};

	ColorImage(CreateInfo);

	// will use as the optimal layout COLOR_ATTACHMENT_OPTIMAL
	// and as the usage COLOR_ATTACHMENT_BIT
	static ColorImage createDrawImage(Device&,
									  VkExtent2D,
									  ColorFormat = ColorFormat::RGBA8);
};

}  // namespace ignis
