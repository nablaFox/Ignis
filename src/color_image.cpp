#include "color_image.hpp"

using namespace ignis;

ColorImage::ColorImage(CreateInfo info)
	: m_format(info.format),
	  Image(*info.device,
			info.extent,
			static_cast<VkFormat>(info.format),
			info.usage,
			info.sampleCount,
			info.optimalLayout,
			VK_IMAGE_ASPECT_COLOR_BIT,
			info.initialPixels) {}

ColorImage* ColorImage::createDrawImage(DrawImageCreateInfo info) {
	return new ColorImage({
		.device = info.device,
		.format = info.format,
		.extent = info.extent,
		.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
				 VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
		.optimalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		.sampleCount = info.sampleCount,
	});
}
