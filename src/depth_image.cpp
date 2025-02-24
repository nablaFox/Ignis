#include "depth_image.hpp"

using namespace ignis;

DepthImage::DepthImage(CreateInfo info)
	: m_format(info.format),
	  Image(*info.device,
			info.extent,
			static_cast<VkFormat>(info.format),
			info.usage,
			info.sampleCount,
			info.optimalLayout,
			VK_IMAGE_ASPECT_DEPTH_BIT,
			info.initialPixels) {}

DepthImage* DepthImage::createDepthImage(DepthStencilCreateInfo info) {
	return new DepthImage({
		.device = info.device,
		.format = info.format,
		.extent = info.extent,
		.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		.optimalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		.sampleCount = info.sampleCount,
	});
}
