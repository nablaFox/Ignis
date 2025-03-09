#include "image.hpp"
#include "command.hpp"
#include "device.hpp"
#include "exceptions.hpp"
#include "fence.hpp"
#include "vk_utils.hpp"

using namespace ignis;

Image::Image(const Device& device, const ImageCreateInfo& info)
	: m_device(device),
	  m_pixelSize(::getPixelSize(info.format)),
	  m_creationInfo(info) {
	assert(info.width > 0 && info.height > 0 && "Invalid image extent");

	VkImageCreateInfo imageInfo = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.imageType = VK_IMAGE_TYPE_2D,
		.format = info.format,
		.extent = {info.width, info.height, 1},
		.mipLevels = 1,
		.arrayLayers = 1,
		.samples = info.sampleCount,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = info.usage,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
	};

	VmaAllocationCreateInfo allocationInfo = {
		.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
	};

	THROW_VULKAN_ERROR(
		vmaCreateImage(m_device.getAllocator(), &imageInfo, &allocationInfo,
					   &m_image, &m_allocation, nullptr),
		"Failed to create image");

	VkImageViewCreateInfo viewInfo = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.image = m_image,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format = info.format,
		.subresourceRange =
			{
				.aspectMask = info.aspect,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
	};

	THROW_VULKAN_ERROR(
		vkCreateImageView(m_device.getDevice(), &viewInfo, nullptr, &m_view),
		"Failed to create image view");
}

Image::Image(const Device& device,
			 VkImage image,
			 VkImageView view,
			 const ImageCreateInfo& info)
	: m_image(image),
	  m_view(view),
	  m_device(device),
	  m_creationInfo(info),
	  m_pixelSize(::getPixelSize(info.format)) {
	assert(info.width > 0 && info.height > 0 && "Invalid image extent");
	assert(m_image != nullptr && "Invalid image handle");
}

Image::~Image() {
	// TEMP: now we are using m_view to check if the image was created
	// as a wrapper or as an allocation
	if (m_view != nullptr) {
		vkDestroyImageView(m_device.getDevice(), m_view, nullptr);
		vmaDestroyImage(m_device.getAllocator(), m_image, m_allocation);
	}
}

Image Image::allocateDepthImage(const Device& device,
								const DepthImageCreateInfo& info) {
	ImageCreateInfo imageCreateInfo{
		.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		.aspect = VK_IMAGE_ASPECT_DEPTH_BIT,
		.width = info.width,
		.height = info.height,
		.format = static_cast<VkFormat>(info.format),
		.optimalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		.sampleCount = info.sampleCount,
	};

	return Image(device, imageCreateInfo);
}

Image Image::allocateDrawImage(const Device& device,
							   const DrawImageCreateInfo& info) {
	ImageCreateInfo imageCreateInfo{
		.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
				 VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
		.aspect = VK_IMAGE_ASPECT_COLOR_BIT,
		.width = info.width,
		.height = info.height,
		.format = static_cast<VkFormat>(info.format),
		.optimalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		.sampleCount = info.sampleCount,
	};

	return Image(device, imageCreateInfo);
}
