#pragma once

#include <vulkan/vulkan_core.h>
#include <vk_mem_alloc.h>

namespace ignis {

class Device;

// Note 1: we don't support custom views. Every image has a fixed view
// Note 2: we don't allow for multi layer images
// Note 3: we don't allow for multisampled images
// Note 4: we don't allow for 3D images
// Note 5: we don't allow for custom image creation flags
// Note 6: images are never host visible

class Image {
protected:
	Image(const Device&,
		  VkExtent2D,
		  VkFormat,
		  VkImageUsageFlagBits,
		  VkImageLayout optimalLayout,
		  const void* initialPixels);

	~Image();

public:
	VkImageView getView() const { return m_view; }

	VkExtent2D getExtent() const { return m_extent; }

	VkImageLayout getOptimalLayout() const { return m_optimalLayout; }

	VkImageLayout getCurrentLayout() const { return m_currentLayout; }

	VkImageUsageFlagBits getUsage() const { return m_usage; }

private:
	const Device& m_device;
	VmaAllocation m_allocation{nullptr};
	VkImage m_image{nullptr};
	VkImageView m_view{nullptr};
	VkExtent2D m_extent;
	VkFormat m_format;
	VkImageUsageFlagBits m_usage;
	VkImageLayout m_optimalLayout;
	VkImageLayout m_currentLayout;

public:
	Image(const Image&) = delete;
	Image(Image&&) = delete;
	Image& operator=(const Image&) = delete;
	Image& operator=(Image&&) = delete;
};

}  // namespace ignis
