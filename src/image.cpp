#include "image.hpp"
#include "command.hpp"
#include "device.hpp"
#include "exceptions.hpp"
#include "vk_utils.hpp"

using namespace ignis;

Image::Image(VkDevice device, VmaAllocator allocator, const ImageCreateInfo& info)
	: m_device(device),
	  m_allocator(allocator),
	  m_pixelSize(::getPixelSize(info.format)),
	  m_creationInfo(info) {
	assert(m_device && "Invalid device");
	assert(m_allocator && "Invalid allocator");
	assert(info.width > 0 && info.height > 0 && "Invalid image extent");

	VkImageCreateInfo const imageInfo{
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

	VmaAllocationCreateInfo const allocationInfo{
		.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
	};

	THROW_VULKAN_ERROR(vmaCreateImage(m_allocator, &imageInfo, &allocationInfo,
									  &m_image, &m_allocation, nullptr),
					   "Failed to create image");

	VkImageViewCreateInfo viewInfo{
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

	THROW_VULKAN_ERROR(vkCreateImageView(m_device, &viewInfo, nullptr, &m_view),
					   "Failed to create image view");
}

Image::Image(VkImage image, VkImageView view, const ImageCreateInfo& info)
	: m_image(image),
	  m_view(view),
	  m_creationInfo(info),
	  m_pixelSize(::getPixelSize(info.format)) {
	assert(info.width > 0 && info.height > 0 && "Invalid image extent");
	assert(m_image != nullptr && "Invalid image handle");
}

Image::~Image() {
	if (m_allocator == nullptr || m_device == nullptr) {
		return;
	}

	vkDestroyImageView(m_device, m_view, nullptr);
	vmaDestroyImage(m_allocator, m_image, m_allocation);
}

Image::Image(Image&& other) noexcept
	: m_device(other.m_device),
	  m_allocator(other.m_allocator),
	  m_image(other.m_image),
	  m_view(other.m_view),
	  m_pixelSize(other.m_pixelSize),
	  m_creationInfo(other.m_creationInfo),
	  m_allocation(other.m_allocation) {
	other.m_image = VK_NULL_HANDLE;
	other.m_view = VK_NULL_HANDLE;
	other.m_allocation = VK_NULL_HANDLE;
	other.m_allocator = nullptr;
	other.m_device = nullptr;
}

ImageCreateInfo Image::depthImageDesc(const DepthImageCreateInfo& info) {
	return {
		.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		.aspect = VK_IMAGE_ASPECT_DEPTH_BIT,
		.width = info.width,
		.height = info.height,
		.format = static_cast<VkFormat>(info.format),
		.optimalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		.sampleCount = info.sampleCount,
	};
}

ImageCreateInfo Image::drawImageDesc(const DrawImageCreateInfo& info) {
	return {
		.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
				 VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
		.aspect = VK_IMAGE_ASPECT_COLOR_BIT,
		.width = info.width,
		.height = info.height,
		.format = static_cast<VkFormat>(info.format),
		.optimalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		.sampleCount = info.sampleCount,
	};
}
