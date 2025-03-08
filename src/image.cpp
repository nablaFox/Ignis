#include <cassert>
#include "exceptions.hpp"
#include "image.hpp"
#include "vk_utils.hpp"

using namespace ignis;

Image::Image(VkDevice device,
			 VkImage image,
			 VkImageView view,
			 VmaAllocation allocation,
			 VmaAllocator allocator,
			 const CreateInfo& info)
	: m_device(device),
	  m_image(image),
	  m_view(view),
	  m_allocation(allocation),
	  m_allocator(allocator),
	  m_creationInfo(info),
	  m_pixelSize(::getPixelSize(info.format)),
	  m_currentLayout(VK_IMAGE_LAYOUT_UNDEFINED) {
	assert(device != nullptr && "Invalid device");
	assert(image != nullptr && "Invalid image");
	assert(view != nullptr && "Invalid image view");
	assert(allocation != nullptr && "Invalid allocation");
	assert(allocator != nullptr && "Invalid allocator");
	assert(m_pixelSize > 0 && "Invalid pixel size");
	assert(info.width > 0 && info.height > 0 && "Invalid image extent");
}

Image::Image(VkImage image, VkImageView view, const CreateInfo& info)
	: Image(nullptr, image, view, nullptr, nullptr, info) {}

Image::~Image() {
	if (m_allocator == nullptr || m_device == nullptr)
		return;

	vkDestroyImageView(m_device, m_view, nullptr);
	vmaDestroyImage(m_allocator, m_image, m_allocation);
}

Image Image::allocateImage(VkDevice device,
						   VmaAllocator allocator,
						   const CreateInfo& info) {
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

	VkImage image{nullptr};
	VmaAllocation allocation{nullptr};

	THROW_VULKAN_ERROR(vmaCreateImage(allocator, &imageInfo, &allocationInfo, &image,
									  &allocation, nullptr),
					   "Failed to create image");

	VkImageView view{nullptr};

	VkImageViewCreateInfo viewInfo = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.image = image,
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

	THROW_VULKAN_ERROR(vkCreateImageView(device, &viewInfo, nullptr, &view),
					   "Failed to create image view");

	return Image(device, image, view, allocation, allocator, info);
}
