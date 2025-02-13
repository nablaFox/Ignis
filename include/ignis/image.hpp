#pragma once

#include <vulkan/vulkan_core.h>

namespace ignis {

class Device;

// Note 1: we don't support custom views. Every image has a fixed view
// Note 2: we don't allow for multi layer images
// Note 3: we don't allow for multisampled images
// Note 4: we don't allow for 3D images
// Note 5: we don't allow for custom image creation flags

class Image {
protected:
	Image(Device&,
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
	VkImage m_image;
	VkImageView m_view;
	VkExtent2D m_extent;
	VkImageUsageFlagBits m_usage;
	VkImageLayout m_optimalLayout;
	VkImageLayout m_currentLayout;

public:
	Image(const Image&) = default;
	Image(Image&&) = delete;
	Image& operator=(const Image&) = default;
	Image& operator=(Image&&) = delete;
};

}  // namespace ignis
