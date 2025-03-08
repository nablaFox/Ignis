#pragma once

#include <vulkan/vulkan_core.h>
#include <vk_mem_alloc.h>

namespace ignis {

class Device;

struct ImageCreateInfo {
	VkImageUsageFlags usage{VK_IMAGE_USAGE_TRANSFER_DST_BIT |
							VK_IMAGE_USAGE_SAMPLED_BIT};
	VkImageAspectFlags aspect{VK_IMAGE_ASPECT_COLOR_BIT};
	uint32_t width{0};
	uint32_t height{0};
	VkFormat format{VK_FORMAT_R8G8B8A8_UNORM};
	VkImageLayout optimalLayout{VK_IMAGE_LAYOUT_UNDEFINED};
	VkSampleCountFlagBits sampleCount{VK_SAMPLE_COUNT_1_BIT};
	const void* initialPixels{nullptr};
};

struct Image {
	friend class Command;

public:
	// wrapper
	Image(const Device&, VkImage, VkImageView, const ImageCreateInfo&);

	// gpu allocated
	Image(const Device&, const ImageCreateInfo&);

	~Image();

	VkImage getHandle() const { return m_image; }

	VkImageUsageFlags getUsage() const { return m_creationInfo.usage; }

	VkImageAspectFlags getAspect() const { return m_creationInfo.aspect; }

	VkImageLayout getOptimalLayout() const { return m_creationInfo.optimalLayout; }

	VkImageLayout getCurrentLayout() const { return m_currentLayout; }

	VkExtent3D getExtent() const {
		return {m_creationInfo.width, m_creationInfo.height, 1};
	}

	VkExtent2D getExtent2D() const {
		return {m_creationInfo.width, m_creationInfo.height};
	}

	VkFormat getFormat() const { return m_creationInfo.format; }

	VkDeviceSize getPixelSize() const { return m_pixelSize; }

	VkDeviceSize getSize() const {
		return static_cast<VkDeviceSize>(m_creationInfo.width *
										 m_creationInfo.height) *
			   m_pixelSize;
	}

	VkImageView getViewHandle() const { return m_view; }

	VkSampleCountFlagBits getSampleCount() const {
		return m_creationInfo.sampleCount;
	}

private:
	const Device& m_device;
	VmaAllocation m_allocation{nullptr};
	VkImage m_image{nullptr};
	VkImageView m_view{nullptr};
	VkImageLayout m_currentLayout{VK_IMAGE_LAYOUT_UNDEFINED};
	VkDeviceSize m_pixelSize;
	ImageCreateInfo m_creationInfo;

public:
	Image(const Image&) = delete;
	Image& operator=(const Image&) = delete;
	Image(Image&&) = delete;
	Image& operator=(Image&&) = delete;
};

}  // namespace ignis
