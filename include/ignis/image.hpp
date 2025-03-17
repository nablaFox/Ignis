#pragma once

#include <vk_mem_alloc.h>

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

struct ImageCreateInfo {
	VkImageUsageFlags usage{0};
	VkImageAspectFlags aspect{VK_IMAGE_ASPECT_NONE};
	uint32_t width{0};
	uint32_t height{0};
	VkFormat format{VK_FORMAT_UNDEFINED};
	VkImageLayout optimalLayout{VK_IMAGE_LAYOUT_UNDEFINED};
	VkSampleCountFlagBits sampleCount{VK_SAMPLE_COUNT_1_BIT};
};

struct DepthImageCreateInfo {
	uint32_t width{0};
	uint32_t height{0};
	DepthFormat format{DepthFormat::D16_UNORM};
	VkSampleCountFlagBits sampleCount{VK_SAMPLE_COUNT_1_BIT};
};

struct DrawImageCreateInfo {
	uint32_t width{0};
	uint32_t height{0};
	ColorFormat format{ColorFormat::RGBA16};
	VkSampleCountFlagBits sampleCount{VK_SAMPLE_COUNT_1_BIT};
};

struct Image {
	friend class Command;

public:
	// wrapper
	Image(VkImage, VkImageView, const ImageCreateInfo&);

	// gpu allocated (the allocator should be relative to the device passed here)
	Image(VkDevice, VmaAllocator, const ImageCreateInfo&);

	~Image();

	auto getHandle() const { return m_image; }

	auto getUsage() const { return m_creationInfo.usage; }

	auto getAspect() const { return m_creationInfo.aspect; }

	auto getOptimalLayout() const { return m_creationInfo.optimalLayout; }

	auto getCurrentLayout() const { return m_currentLayout; }

	auto getExtent() const -> VkExtent3D {
		return {m_creationInfo.width, m_creationInfo.height, 1};
	}

	auto getExtent2D() const -> VkExtent2D {
		return {m_creationInfo.width, m_creationInfo.height};
	}

	auto getFormat() const { return m_creationInfo.format; }

	auto getPixelSize() const { return m_pixelSize; }

	auto getSize() const {
		return static_cast<VkDeviceSize>(m_creationInfo.width *
										 m_creationInfo.height) *
			   m_pixelSize;
	}

	auto getViewHandle() const { return m_view; }

	auto getSampleCount() const { return m_creationInfo.sampleCount; }

public:
	static Image allocateDrawImage(VkDevice,
								   VmaAllocator,
								   const DrawImageCreateInfo&);

	static Image allocateDepthImage(VkDevice,
									VmaAllocator,
									const DepthImageCreateInfo&);

private:
	VkDevice m_device{nullptr};
	VmaAllocator m_allocator{nullptr};
	VmaAllocation m_allocation{nullptr};
	VkImage m_image{nullptr};
	VkImageView m_view{nullptr};
	VkImageLayout m_currentLayout{VK_IMAGE_LAYOUT_UNDEFINED};
	VkDeviceSize m_pixelSize;
	ImageCreateInfo m_creationInfo;

public:
	Image(Image&&) noexcept;
	Image& operator=(const Image&) = delete;
	Image(const Image&) = delete;
	Image& operator=(Image&&) = delete;
};

}  // namespace ignis
