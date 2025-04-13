#pragma once

#include <vulkan/vulkan_core.h>

namespace ignis {

class Device;

class Sampler {
public:
	struct SamplerInfo {
		const Device* device{nullptr};
		VkFilter m_magFilter{VK_FILTER_NEAREST};
		VkFilter m_minFilter{VK_FILTER_NEAREST};
		VkSamplerMipmapMode m_mipmapMode{VK_SAMPLER_MIPMAP_MODE_NEAREST};
		VkSamplerAddressMode m_addressModeU{VK_SAMPLER_ADDRESS_MODE_REPEAT};
		VkSamplerAddressMode m_addressModeV{VK_SAMPLER_ADDRESS_MODE_REPEAT};
		VkSamplerAddressMode m_addressModeW{VK_SAMPLER_ADDRESS_MODE_REPEAT};
		float m_mipLodBias{0.0f};
		VkBool32 m_anisotropyEnable{VK_FALSE};
		float m_maxAnisotropy{1.0f};
		VkBool32 m_compareEnable{VK_FALSE};
		VkCompareOp m_compareOp{VK_COMPARE_OP_NEVER};
		float m_minLod{0.0f};
		float m_maxLod{0.0f};
		VkBorderColor m_borderColor{VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK};
		VkBool32 m_unnormalizedCoordinates{VK_FALSE};
	};

	Sampler(SamplerInfo info);
	~Sampler();

	VkSampler getHandle() const { return m_sampler; }

private:
	VkSampler m_sampler{VK_NULL_HANDLE};
	SamplerInfo m_info;
};

}  // namespace ignis
