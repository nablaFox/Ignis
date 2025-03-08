#pragma once

#include <vulkan/vulkan_core.h>
#include <vk_mem_alloc.h>

// Note 1: we don't support custom views. Every image has a fixed view
// Note 2: we don't allow for multi layer images
// Note 3: we don't allow for 3D images
// Note 4: we don't allow for custom image creation flags
// Note 5: images are never host visible

namespace ignis {

enum class DepthFormat {
	D16_UNORM = VK_FORMAT_D16_UNORM,
	D24_UNORM_S8_UINT = VK_FORMAT_D24_UNORM_S8_UINT,
	D32_SFLOAT = VK_FORMAT_D32_SFLOAT
};

enum class ColorFormat {
	RGBA8 = VK_FORMAT_R8G8B8A8_UNORM,
	RGBA16 = VK_FORMAT_R16G16B16A16_SFLOAT,
	HDR = VK_FORMAT_R32G32B32A32_SFLOAT,
};

class Image {
	friend class Command;

public:
	struct CreateInfo {
		VkImageUsageFlags usage{VK_IMAGE_USAGE_TRANSFER_DST_BIT |
								VK_IMAGE_USAGE_SAMPLED_BIT};
		VkImageAspectFlags aspect{VK_IMAGE_ASPECT_COLOR_BIT};
		uint32_t width{0};
		uint32_t height{0};
		VkFormat format{VK_FORMAT_R8G8B8A8_UNORM};
		VkImageLayout optimalLayout{VK_IMAGE_LAYOUT_UNDEFINED};
		VkSampleCountFlagBits sampleCount{VK_SAMPLE_COUNT_1_BIT};
	};

	// wrapper
	Image(VkImage, VkImageView, const CreateInfo&);

	// gpu allocated
	Image(VkDevice, VmaAllocator, const CreateInfo&);

	~Image();

	static Image allocateImage(VkDevice, VmaAllocator, const CreateInfo&);

	VkDeviceSize getPixelSize() const { return m_pixelSize; }

	VkDeviceSize getImageSize() const {
		return static_cast<VkDeviceSize>(m_creationInfo.width *
										 m_creationInfo.height) *
			   m_pixelSize;
	}

	VkImage getHandle() const { return m_image; }

	VkImageView getViewHandle() const { return m_view; }

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

	VkSampleCountFlagBits getSampleCount() const {
		return m_creationInfo.sampleCount;
	}

private:
	VkDevice m_device{nullptr};
	VmaAllocator m_allocator{nullptr};

	VmaAllocation m_allocation{nullptr};
	VkImage m_image{nullptr};
	VkImageView m_view{nullptr};

	VkImageLayout m_currentLayout;
	VkDeviceSize m_pixelSize;
	CreateInfo m_creationInfo;

public:
	Image(const Image&) = delete;
	Image& operator=(const Image&) = delete;
	Image(Image&&) = default;
	Image& operator=(Image&&) = delete;
};

}  // namespace ignis
