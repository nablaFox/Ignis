#pragma once

#include <vulkan/vulkan_core.h>
#include <vector>

namespace ignis {

class Device;

struct PipelineBinding {
	VkDescriptorType descriptorType;
	VkShaderStageFlags stages;
	VkAccessFlags access;
	uint32_t binding;
	uint32_t arraySize;
	uint32_t size;
};

class DescriptorSetLayout {
public:
	DescriptorSetLayout(Device& m_device, std::vector<PipelineBinding>);
	~DescriptorSetLayout();

private:
	Device& m_device;
	VkDescriptorSetLayout m_descriptorSetLayout;

public:
	DescriptorSetLayout(const DescriptorSetLayout&) = delete;
	DescriptorSetLayout(DescriptorSetLayout&&) = delete;
	DescriptorSetLayout& operator=(const DescriptorSetLayout&) = delete;
	DescriptorSetLayout& operator=(DescriptorSetLayout&&) = delete;
};

}  // namespace ignis
