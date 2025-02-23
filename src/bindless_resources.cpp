#include "bindless_resources.hpp"
#include <vector>
#include "exceptions.hpp"

using namespace ignis;

void BindlessResources::initialize(VkDevice device) {
	// TEMP
	constexpr uint32_t maxUbos = 1000;
	constexpr uint32_t maxSsbos = 1000;
	constexpr uint32_t maxImageSamplers = 1000;
	//

	VkDescriptorSetLayoutBinding const uboBindings = {
		.binding = IGNIS_UNIFORM_BUFFER_BINDING,
		.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		.descriptorCount = maxUbos,
		.stageFlags = VK_SHADER_STAGE_ALL,
		.pImmutableSamplers = nullptr,
	};

	VkDescriptorSetLayoutBinding const ssboBindings = {
		.binding = IGNIS_STORAGE_BUFFER_BINDING,
		.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		.descriptorCount = maxSsbos,
		.stageFlags = VK_SHADER_STAGE_ALL,
		.pImmutableSamplers = nullptr,
	};

	VkDescriptorSetLayoutBinding const imageSamplerBindings = {
		.binding = IGNIS_IMAGE_SAMPLER_BINDING,
		.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		.descriptorCount = maxImageSamplers,
		.stageFlags = VK_SHADER_STAGE_ALL,
		.pImmutableSamplers = nullptr,
	};

	auto bindings = std::vector<VkDescriptorSetLayoutBinding>{
		uboBindings,
		ssboBindings,
		imageSamplerBindings,
	};

	auto bindingFlags = std::vector{
		VkDescriptorBindingFlags{VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
								 VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT},
		VkDescriptorBindingFlags{VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
								 VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT},
		VkDescriptorBindingFlags{VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
								 VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT},
		VkDescriptorBindingFlags{VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
								 VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT},
		VkDescriptorBindingFlags{VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
								 VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT},
	};

	VkDescriptorSetLayoutBindingFlagsCreateInfo const bindingFlagsCreateInfo{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
		.pNext = nullptr,
		.bindingCount = 3,
		.pBindingFlags = bindingFlags.data(),
	};

	VkDescriptorSetLayoutCreateInfo const createInfo{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.pNext = &bindingFlagsCreateInfo,
		.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
		.bindingCount = 3,
		.pBindings = bindings.data(),
	};

	THROW_VULKAN_ERROR(vkCreateDescriptorSetLayout(device, &createInfo, nullptr,
												   &m_descriptorSetLayout),
					   "Failed to create descriptor set layout");

	// create descriptor pool
	VkDescriptorPoolSize const texturePoolSize{
		.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		.descriptorCount = maxUbos,
	};

	VkDescriptorPoolSize const uboPoolSize{
		.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		.descriptorCount = maxImageSamplers,
	};

	VkDescriptorPoolSize const ssboPoolSize{
		.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		.descriptorCount = maxSsbos,
	};

	auto poolSizes = std::vector<VkDescriptorPoolSize>{
		texturePoolSize,
		uboPoolSize,
		ssboPoolSize,
	};

	VkDescriptorPoolCreateInfo const vk_descriptor_pool_create_info{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.pNext = nullptr,
		.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT |
				 VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
		.maxSets = 1,
		.poolSizeCount = 3,
		.pPoolSizes = poolSizes.data(),
	};

	THROW_VULKAN_ERROR(
		vkCreateDescriptorPool(device, &vk_descriptor_pool_create_info, nullptr,
							   &m_descriptorPool),
		"Failed to create descriptor pool");

	// create descriptor set
	VkDescriptorSetAllocateInfo const descriptorSetAllocateInfo{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.pNext = nullptr,
		.descriptorPool = m_descriptorPool,
		.descriptorSetCount = 1,
		.pSetLayouts = &m_descriptorSetLayout,
	};

	THROW_VULKAN_ERROR(vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo,
												&m_descriptorSet),
					   "Failed to allocate descriptor set");

	// create pipeline layout without push constants
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.pNext = nullptr,
		.flags = {},
		.setLayoutCount = 1,
		.pSetLayouts = &m_descriptorSetLayout,
		.pushConstantRangeCount = 0,
		.pPushConstantRanges = nullptr,
	};

	THROW_VULKAN_ERROR(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr,
											  m_pipelineLayouts.data()),
					   "Failed to create pipeline layout");

	// create other pipeline layouts
	for (uint32_t i = 1; i < MAX_PUSH_CONSTANT_WORD_SIZE; ++i) {
		VkPushConstantRange const pushConstantRange{
			.stageFlags = VK_SHADER_STAGE_ALL,
			.offset = 0,
			.size = i * 4,
		};

		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

		THROW_VULKAN_ERROR(
			vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr,
								   m_pipelineLayouts.data() + i + 1),
			"Failed to create pipeline layout");
	}
}

void BindlessResources::cleanup(VkDevice device) {
	vkDestroyDescriptorSetLayout(device, m_descriptorSetLayout, nullptr);
	vkDestroyDescriptorPool(device, m_descriptorPool, nullptr);

	for (auto layout : m_pipelineLayouts) {
		vkDestroyPipelineLayout(device, layout, nullptr);
	}
}
