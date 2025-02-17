#include "pipeline_layout.hpp"
#include "shader.hpp"
#include "device.hpp"
#include "descriptor_set_layout.hpp"
#include "exceptions.hpp"

using namespace ignis;

PipelineLayout::PipelineLayout(const Device& device,
							   const std::vector<std::unique_ptr<Shader>>& shaders)
	: m_device(device) {
	ShaderResources shaderResources{};

	for (const auto& shader : shaders)
		Shader::getMergedResources(shader->getResources(), &shaderResources);

	m_pushConstantsRange = shaderResources.pushConstants;

	std::vector<VkDescriptorSetLayout> vkDescriptorSetLayouts;
	vkDescriptorSetLayouts.reserve(shaderResources.bindings.size());

	m_descriptorSetLayouts.reserve(shaderResources.bindings.size());

	for (const auto& [slot, bindings] : shaderResources.bindings) {
		auto [it, inserted] =
			m_descriptorSetLayouts.try_emplace(slot, m_device, bindings);

		vkDescriptorSetLayouts.push_back(it->second.getHandle());
	}

	VkPipelineLayoutCreateInfo layoutInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.setLayoutCount = static_cast<uint32_t>(vkDescriptorSetLayouts.size()),
		.pSetLayouts = vkDescriptorSetLayouts.data(),
		.pushConstantRangeCount =
			static_cast<uint32_t>((shaderResources.pushConstants.size > 0) ? 1 : 0),
		.pPushConstantRanges = (shaderResources.pushConstants.size > 0)
								   ? &shaderResources.pushConstants
								   : nullptr,
	};

	THROW_VULKAN_ERROR(vkCreatePipelineLayout(m_device.getDevice(), &layoutInfo,
											  nullptr, &m_layout),
					   "Failed to create pipeline layout");
}

PipelineLayout::~PipelineLayout() {
	vkDestroyPipelineLayout(m_device.getDevice(), m_layout, nullptr);
}

const BindingInfo& PipelineLayout::getBindingInfo(uint32_t slot,
												  uint32_t binding) const {
	auto it = m_descriptorSetLayouts.find(slot);

	THROW_ERROR(it == m_descriptorSetLayouts.end(),
				"Slot not found" + std::to_string(slot));

	return it->second.getBindingInfo(binding);
}
