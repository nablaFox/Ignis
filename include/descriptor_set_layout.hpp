#pragma once

#include <vulkan/vulkan_core.h>
#include <vector>

namespace ignis {

class Device;

struct BindingInfo {
	VkDescriptorType bindingType;
	VkShaderStageFlags stages;
	VkAccessFlags access;
	uint32_t binding;
	uint32_t arraySize;
	uint32_t size;
};

class DescriptorSetLayout {
public:
	DescriptorSetLayout(Device&, std::vector<BindingInfo>);
	~DescriptorSetLayout();

	VkDescriptorSetLayout getHandle() const { return m_descriptorSetLayout; }

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
