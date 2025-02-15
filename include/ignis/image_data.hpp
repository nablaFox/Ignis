#pragma once

#include <vulkan/vulkan_core.h>

namespace ignis {

struct ImageData {
	friend class Command;

public:
	ImageData(VkImage handle,
			  VkImageUsageFlags usage,
			  VkImageAspectFlags aspect,
			  VkExtent2D extent,
			  VkFormat format,
			  VkImageLayout currentLayout,
			  VkImageLayout optimalLayout)
		: m_handle(handle),
		  m_usage(usage),
		  m_aspect(aspect),
		  m_extent(extent),
		  m_format(format),
		  m_currentLayout(currentLayout),
		  m_optimalLayout(optimalLayout) {}

	VkImage getHandle() const { return m_handle; }

	VkImageUsageFlags getUsage() const { return m_usage; }

	VkImageAspectFlags getAspect() const { return m_aspect; }

	VkImageLayout getOptimalLayout() const { return m_optimalLayout; }

	VkImageLayout getCurrentLayout() const { return m_currentLayout; }

	VkExtent2D getExtent() const { return m_extent; }

protected:
	VkImage m_handle;
	VkImageUsageFlags m_usage;
	VkImageAspectFlags m_aspect;
	VkExtent2D m_extent;
	VkFormat m_format;
	VkImageLayout m_currentLayout;
	VkImageLayout m_optimalLayout;
};

}  // namespace ignis
