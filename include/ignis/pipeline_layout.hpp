#pragma once

#include <vulkan/vulkan_core.h>
#include <memory>
#include <unordered_map>
#include <vector>

namespace ignis {

class Device;
class DescriptorSetLayout;
class Shader;

class PipelineLayout {
public:
	PipelineLayout(Device&, const std::vector<std::unique_ptr<Shader>>& /* (sus) */);
	~PipelineLayout();

	VkPipelineLayout getHandle() { return m_layout; }

private:
	Device& m_device;
	std::unordered_map<uint32_t, DescriptorSetLayout> m_descriptorSetLayouts;
	VkPipelineLayout m_layout{VK_NULL_HANDLE};

public:
	PipelineLayout(const PipelineLayout&) = delete;
	PipelineLayout(PipelineLayout&&) = delete;
	PipelineLayout& operator=(const PipelineLayout&) = delete;
	PipelineLayout& operator=(PipelineLayout&&) = delete;
};

}  // namespace ignis
