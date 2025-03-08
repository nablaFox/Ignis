#include <cassert>
#include "exceptions.hpp"
#include "image.hpp"
#include "vk_utils.hpp"

using namespace ignis;

Image::Image(VkDevice device, VmaAllocator allocator, const CreateInfo& info)
	: m_device(device),
	  m_allocator(allocator),
	  m_creationInfo(info),
	  m_pixelSize(::getPixelSize(info.format)),
	  m_currentLayout(VK_IMAGE_LAYOUT_UNDEFINED) {
	assert(device != nullptr && "Invalid device");
	assert(allocator != nullptr && "Invalid allocator");
	assert(m_pixelSize > 0 && "Invalid pixel size");
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

	THROW_VULKAN_ERROR(vmaCreateImage(allocator, &imageInfo, &allocationInfo,
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

	THROW_VULKAN_ERROR(vkCreateImageView(device, &viewInfo, nullptr, &m_view),
					   "Failed to create image view");
}

Image::Image(VkImage image, VkImageView view, const CreateInfo& info)
	: m_image(image),
	  m_view(view),
	  m_creationInfo(info),
	  m_pixelSize(::getPixelSize(info.format)),
	  m_currentLayout(VK_IMAGE_LAYOUT_UNDEFINED) {
	assert(image != nullptr && "Invalid image");
	assert(m_pixelSize > 0 && "Invalid pixel size");
	assert(info.width > 0 && info.height > 0 && "Invalid image extent");
}

Image::~Image() {
	if (m_allocator == nullptr || m_device == nullptr)
		return;

	vkDestroyImageView(m_device, m_view, nullptr);
	vmaDestroyImage(m_allocator, m_image, m_allocation);
}
