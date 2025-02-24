#pragma once

#include <vulkan/vulkan_core.h>
#include <vk_mem_alloc.h>
#include "image_data.hpp"

namespace ignis {

class Device;

// Note 1: we don't support custom views. Every image has a fixed view
// Note 2: we don't allow for multi layer images
// Note 3: we don't allow for 3D images
// Note 4: we don't allow for custom image creation flags
// Note 5: images are never host visible

class Image : public ImageData {
protected:
	Image(const Device&,
		  VkExtent2D,
		  VkFormat,
		  VkImageUsageFlags,
		  VkSampleCountFlagBits,
		  VkImageLayout optimalLayout,
		  VkImageAspectFlags viewAspect,
		  const void* initialPixels);

	~Image();

public:
	VkImageView getViewHandle() const { return m_view; }

	VkDeviceSize getPixelSize() const { return m_pixelSize; }

	VkSampleCountFlagBits getSampleCount() const { return m_sampleCount; }

private:
	const Device& m_device;
	VmaAllocation m_allocation{nullptr};
	VkImageView m_view{nullptr};
	VkDeviceSize m_pixelSize;
	VkSampleCountFlagBits m_sampleCount;

public:
	Image(const Image&) = delete;
	Image(Image&&) = delete;
	Image& operator=(const Image&) = delete;
	Image& operator=(Image&&) = delete;
};

}  // namespace ignis
