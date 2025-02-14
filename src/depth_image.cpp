#include "depth_image.hpp"

using namespace ignis;

DepthImage::DepthImage(CreateInfo info)
	: Image(*info.device,
			info.extent,
			static_cast<VkFormat>(info.format),
			info.usage,
			info.optimalLayout,
			VK_IMAGE_ASPECT_DEPTH_BIT,
			info.initialPixels) {}

DepthImage* DepthImage::createDepthStencilImage(const Device* device,
												VkExtent2D extent) {
	return new DepthImage({
		.device = device,
		.format = DepthFormat::D32_SFLOAT,
		.extent = extent,
		.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		.optimalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		.initialPixels = nullptr,
	});
}
