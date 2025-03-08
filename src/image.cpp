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
	  m_usage(info.usage),
	  m_aspect(info.aspect),
	  m_pixelSize(::getPixelSize(info.format)),
	  m_extent(info.extent),
	  m_format(info.format),
	  m_sampleCount(info.sampleCount),
	  m_currentLayout(info.currentLayout),
	  m_optimalLayout(info.optimalLayout) {
	assert(m_extent.width > 0 && m_extent.height > 0 && "Invalid image extent");
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
		.extent = {info.extent.width, info.extent.height, 1},
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

// Image::Image(VkDevice device,
// 			 VkExtent2D extent,
// 			 VkFormat format,
// 			 VkImageUsageFlags usage,
// 			 VkSampleCountFlagBits sampleCount,
// 			 VkImageLayout optimalLayout,
// 			 VkImageAspectFlags viewAspect,
// 			 const void* initialPixels)
// 	: ImageData{VK_NULL_HANDLE, usage,	viewAspect,
// 				extent,			format, VK_IMAGE_LAYOUT_UNDEFINED,
// 				optimalLayout},
// 	  m_device(device),
// 	  m_pixelSize(::getPixelSize(format)),
// 	  m_sampleCount(sampleCount) {
// 	VkImageCreateInfo imageInfo = {
// 		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
// 		.imageType = VK_IMAGE_TYPE_2D,
// 		.format = format,
// 		.extent = {extent.width, extent.height, 1},
// 		.mipLevels = 1,
// 		.arrayLayers = 1,
// 		.samples = sampleCount,
// 		.tiling = VK_IMAGE_TILING_OPTIMAL,
// 		.usage = usage,
// 		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
// 		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
// 	};
//
// 	VmaAllocationCreateInfo allocationInfo = {
// 		.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
// 	};
//
// 	THROW_VULKAN_ERROR(
// 		vmaCreateImage(m_device.getAllocator(), &imageInfo, &allocationInfo,
// 					   &m_handle, &m_allocation, nullptr),
// 		"Failed to create image");
//
// 	VkImageViewCreateInfo viewInfo = {
// 		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
// 		.image = m_handle,
// 		.viewType = VK_IMAGE_VIEW_TYPE_2D,
// 		.format = format,
// 		.subresourceRange =
// 			{
// 				.aspectMask = viewAspect,
// 				.baseMipLevel = 0,
// 				.levelCount = 1,
// 				.baseArrayLayer = 0,
// 				.layerCount = 1,
// 			},
// 	};
//
// 	THROW_VULKAN_ERROR(
// 		vkCreateImageView(m_device.getDevice(), &viewInfo, nullptr, &m_view),
// 		"Failed to create image view");
//
// 	Command cmd(m_device);
//
// 	cmd.begin();
//
// 	if (initialPixels) {
// 		cmd.transitionImageLayout(*this, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
// 		cmd.updateImage(*this, initialPixels);
// 	}
//
// 	cmd.transitionImageLayout(*this, m_optimalLayout);
//
// 	cmd.end();
//
// 	Fence fence(m_device);
//
// 	m_device.submitCommands({{&cmd}}, fence);
//
// 	fence.wait();
// }
//
