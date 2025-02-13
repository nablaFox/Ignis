#include "image.hpp"
#include "command.hpp"
#include "device.hpp"
#include "exceptions.hpp"

using namespace ignis;

Image::Image(Device& device,
			 VkExtent2D extent,
			 VkFormat format,
			 VkImageUsageFlagBits usage,
			 VkImageLayout optimalLayout,
			 const void* initialPixels)
	: m_device(device),
	  m_extent(extent),
	  m_usage(usage),
	  m_optimalLayout(optimalLayout),
	  m_currentLayout(optimalLayout),
	  m_format(format) {
	VkImageCreateInfo imageInfo = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.imageType = VK_IMAGE_TYPE_2D,
		.format = format,
		.extent = {extent.width, extent.height, 1},
		.mipLevels = 1,
		.arrayLayers = 1,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = usage,
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

	// create default view
	VkImageViewCreateInfo viewInfo = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.image = m_image,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format = format,
		.subresourceRange =
			{
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
	};

	THROW_VULKAN_ERROR(
		vkCreateImageView(m_device.getDevice(), &viewInfo, nullptr, &m_view),
		"Failed to create image view");

	Command cmd(m_device);

	cmd.updateImage(*this, initialPixels);

	cmd.transitionImageLayout(*this, m_optimalLayout);

	m_device.submitCommands({{
		.command = &cmd,
	}});
}

Image::~Image() {
	vkDestroyImageView(m_device.getDevice(), m_view, nullptr);
	vmaDestroyImage(m_device.getAllocator(), m_image, m_allocation);
}
