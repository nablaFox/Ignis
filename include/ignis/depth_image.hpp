#pragma once

#include "image.hpp"

namespace ignis {

enum class DepthFormat {
	D16_UNORM = VK_FORMAT_D16_UNORM,
	D24_UNORM_S8_UINT = VK_FORMAT_D24_UNORM_S8_UINT,
	D32_SFLOAT = VK_FORMAT_D32_SFLOAT
};

class DepthImage : public Image {
public:
	struct CreateInfo {
		const Device* device{nullptr};
		DepthFormat format{DepthFormat::D32_SFLOAT};
		VkExtent2D extent{0, 0};
		VkImageUsageFlags usage{};
		VkImageLayout optimalLayout{VK_IMAGE_LAYOUT_UNDEFINED};
		VkSampleCountFlagBits sampleCount{VK_SAMPLE_COUNT_1_BIT};
		const void* initialPixels{nullptr};
	};

	DepthImage(CreateInfo);

	struct DepthStencilCreateInfo {
		const Device* device{nullptr};
		VkExtent2D extent{0, 0};
		DepthFormat format{DepthFormat::D32_SFLOAT};
		VkSampleCountFlagBits sampleCount{VK_SAMPLE_COUNT_1_BIT};
	};

	static DepthImage* createDepthImage(DepthStencilCreateInfo);

	DepthFormat getFormat() const { return m_format; }

private:
	DepthFormat m_format;
};

}  // namespace ignis
