#pragma once

#include "image.hpp"

namespace ignis {

enum class ColorFormat {
	RGBA8 = VK_FORMAT_R8G8B8A8_UNORM,
	RGBA16 = VK_FORMAT_R16G16B16A16_SFLOAT,
	HDR = VK_FORMAT_R32G32B32A32_SFLOAT,
};

class ColorImage : public Image {
public:
	struct CreateInfo {
		const Device* device{nullptr};
		ColorFormat format{ColorFormat::RGBA8};
		VkExtent2D extent{0, 0};
		VkImageUsageFlags usage{};
		VkImageLayout optimalLayout{VK_IMAGE_LAYOUT_UNDEFINED};
		VkSampleCountFlagBits sampleCount{VK_SAMPLE_COUNT_1_BIT};
		const void* initialPixels{nullptr};
	};

	ColorImage(CreateInfo);

	struct DrawImageCreateInfo {
		const Device* device{nullptr};
		VkExtent2D extent{0, 0};
		ColorFormat format{ColorFormat::RGBA16};
		VkSampleCountFlagBits sampleCount{VK_SAMPLE_COUNT_1_BIT};
	};

	static ColorImage* createDrawImage(DrawImageCreateInfo);

	ColorFormat getFormat() const { return m_format; }

private:
	ColorFormat m_format;
};

}  // namespace ignis
