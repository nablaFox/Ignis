#pragma once

#include <vulkan/vulkan_core.h>
#include <vector>

namespace ignis {

class Device;

struct BindingInfo;

class DescriptorSetLayout {
public:
	DescriptorSetLayout(const Device& device, const std::vector<BindingInfo>&);
	~DescriptorSetLayout();

	VkDescriptorSetLayout getHandle() const { return m_descriptorSetLayout; }

private:
	const Device& m_device;
	VkDescriptorSetLayout m_descriptorSetLayout{VK_NULL_HANDLE};

public:
	DescriptorSetLayout(const DescriptorSetLayout&) = delete;
	DescriptorSetLayout(DescriptorSetLayout&&) = delete;
	DescriptorSetLayout& operator=(const DescriptorSetLayout&) = delete;
	DescriptorSetLayout& operator=(DescriptorSetLayout&&) = delete;
};

}  // namespace ignis
