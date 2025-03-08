#include "depth_image.hpp"

using namespace ignis;

DepthImage::DepthImage(CreateInfo info)
	: m_format(info.format),
	  Image(*info.device,
			{
				.usage = info.usage,
				.aspect = VK_IMAGE_ASPECT_DEPTH_BIT,
				.width = info.extent.width,
				.height = info.extent.height,
				.format = static_cast<VkFormat>(info.format),
				.optimalLayout = info.optimalLayout,
				.sampleCount = info.sampleCount,
				.initialPixels = info.initialPixels,
			}) {}

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
