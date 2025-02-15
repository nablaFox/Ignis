#include "descriptor_set_layout.hpp"
#include "device.hpp"
#include "exceptions.hpp"
#include "shader.hpp"

using namespace ignis;

DescriptorSetLayout::DescriptorSetLayout(const Device& device,
										 const std::vector<BindingInfo>& bindings)
	: m_device(device), m_bindings(bindings) {
	std::vector<VkDescriptorSetLayoutBinding> vkBindings(bindings.size());

	for (size_t i = 0; i < bindings.size(); i++) {
		vkBindings[i] = {
			.binding = bindings[i].binding,
			.descriptorType = bindings[i].bindingType,
			.descriptorCount = bindings[i].arraySize,
			.stageFlags = bindings[i].stages,
			.pImmutableSamplers = nullptr,
		};
	}

	VkDescriptorSetLayoutCreateInfo layoutInfo = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR,
		.bindingCount = static_cast<uint32_t>(vkBindings.size()),
		.pBindings = vkBindings.data(),
	};

	THROW_VULKAN_ERROR(vkCreateDescriptorSetLayout(m_device.getDevice(), &layoutInfo,
												   nullptr, &m_descriptorSetLayout),
					   "Failed to create descriptor set layout");
}

DescriptorSetLayout::~DescriptorSetLayout() {
	vkDestroyDescriptorSetLayout(m_device.getDevice(), m_descriptorSetLayout,
								 nullptr);
}

const BindingInfo& DescriptorSetLayout::getBindingInfo(uint32_t binding) const {
	for (const auto& info : m_bindings) {
		if (info.binding == binding)
			return info;
	}

	THROW_ERROR(true, "Binding not found " + std::to_string(binding));
}
