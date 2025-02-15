#pragma once

#include <vulkan/vulkan_core.h>
#include <memory>
#include <unordered_map>
#include <vector>

namespace ignis {

class Device;
class DescriptorSetLayout;
class Shader;
struct BindingInfo;

class PipelineLayout {
public:
	PipelineLayout(const Device&,
				   const std::vector<std::unique_ptr<Shader>>& /* (sus) */);
	~PipelineLayout();

	VkPipelineLayout getHandle() const { return m_layout; }

	const BindingInfo& getBindingInfo(uint32_t slot, uint32_t binding) const;

private:
	const Device& m_device;
	std::unordered_map<uint32_t, DescriptorSetLayout> m_descriptorSetLayouts;
	VkPipelineLayout m_layout{VK_NULL_HANDLE};

public:
	PipelineLayout(const PipelineLayout&) = delete;
	PipelineLayout(PipelineLayout&&) = delete;
	PipelineLayout& operator=(const PipelineLayout&) = delete;
	PipelineLayout& operator=(PipelineLayout&&) = delete;
};

}  // namespace ignis
