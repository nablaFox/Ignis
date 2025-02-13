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

	// - optimal layout COLOR_ATTACHMENT_OPTIMAL
	// - usage COLOR_ATTACHMENT_BIT
	// - format RGBA16
	static ColorImage* createDrawImage(Device* device, VkExtent2D extent);
};

}  // namespace ignis
