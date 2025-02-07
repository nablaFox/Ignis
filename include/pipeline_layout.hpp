#pragma once

#include <vulkan/vulkan_core.h>
#include <unordered_map>

namespace ignis {

class Device;
class DescriptorSetLayout;

class PipelineLayout {
public:
	PipelineLayout(Device&,
				   std::unordered_map<uint32_t, DescriptorSetLayout>,
				   VkPushConstantRange);
	~PipelineLayout();

private:
	Device& m_device;
	VkPipelineLayout m_layout;
	std::unordered_map<uint32_t, DescriptorSetLayout> m_descriptorSetLayouts;

public:
	PipelineLayout(const PipelineLayout&) = delete;
	PipelineLayout(PipelineLayout&&) = delete;
	PipelineLayout& operator=(const PipelineLayout&) = delete;
	PipelineLayout& operator=(PipelineLayout&&) = delete;
};

}  // namespace ignis
