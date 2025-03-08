#pragma once

#include <vulkan/vulkan_core.h>
#include <vk_mem_alloc.h>

namespace ignis {

class Image {
	friend class Device;
	friend class Command;

public:
	struct CreateInfo {
		VkImageUsageFlags usage;
		VkImageAspectFlags aspect;
		VkExtent2D extent;
		VkFormat format;
		VkImageLayout currentLayout;
		VkImageLayout optimalLayout;
		VkSampleCountFlagBits sampleCount;
	};

	Image(VkImage, VkImageView, const CreateInfo&);

	// gpu allocated
	Image(VkDevice,
		  VkImage,
		  VkImageView,
		  VmaAllocation,
		  VmaAllocator,
		  const CreateInfo&);

	~Image();

	VkDeviceSize getPixelSize() const { return m_pixelSize; }

	VkDeviceSize getImageSize() const {
		return static_cast<VkDeviceSize>(m_extent.width * m_extent.height) *
			   m_pixelSize;
	}

	VkImage getHandle() const { return m_image; }

	VkImageView getViewHandle() const { return m_view; }

	VkImageUsageFlags getUsage() const { return m_usage; }

	VkImageAspectFlags getAspect() const { return m_aspect; }

	VkImageLayout getOptimalLayout() const { return m_optimalLayout; }

	VkImageLayout getCurrentLayout() const { return m_currentLayout; }

	VkExtent2D getExtent() const { return m_extent; }

	VkFormat getFormat() const { return m_format; }

	VkSampleCountFlagBits getSampleCount() const { return m_sampleCount; }

private:
	VkDevice m_device;
	VkImage m_image;
	VkImageView m_view;
	VmaAllocation m_allocation;
	VmaAllocator m_allocator;

	VkImageUsageFlags m_usage;
	VkImageAspectFlags m_aspect;
	VkDeviceSize m_pixelSize;
	VkExtent2D m_extent;
	VkFormat m_format;
	VkImageLayout m_currentLayout;
	VkImageLayout m_optimalLayout;
	VkSampleCountFlagBits m_sampleCount;

	static Image allocateImage(VkDevice, VmaAllocator, const CreateInfo&);

public:
	Image(const Image&) = delete;
	Image& operator=(const Image&) = delete;
	Image(Image&&) = default;
	Image& operator=(Image&&) = delete;
};

}  // namespace ignis
