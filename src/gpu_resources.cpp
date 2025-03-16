#include <cassert>
#include <vector>
#include "gpu_resources.hpp"
#include "exceptions.hpp"

using namespace ignis;

GpuResources::GpuResources(const BindlessResourcesCreateInfo& info)
	: m_creationInfo(info) {
	assert(info.device && "Invalid device");
	assert(info.maxStorageBuffers && "Invalid max storage buffers");
	assert(info.maxUniformBuffers && "Invalid max uniform buffers");
	assert(info.maxImageSamplers && "Invalid max image samplers");

	VkDescriptorSetLayoutBinding const uboBindings = {
		.binding = info.uniformBuffersBinding,
		.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		.descriptorCount = info.maxUniformBuffers,
		.stageFlags = VK_SHADER_STAGE_ALL,
		.pImmutableSamplers = nullptr,
	};

	VkDescriptorSetLayoutBinding const ssboBindings = {
		.binding = info.storageBuffersBinding,
		.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		.descriptorCount = info.maxStorageBuffers,
		.stageFlags = VK_SHADER_STAGE_ALL,
		.pImmutableSamplers = nullptr,
	};

	VkDescriptorSetLayoutBinding const imageSamplerBindings = {
		.binding = info.imageSamplersBinding,
		.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		.descriptorCount = info.maxImageSamplers,
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

	THROW_VULKAN_ERROR(vkCreateDescriptorSetLayout(info.device, &createInfo, nullptr,
												   &m_descriptorSetLayout),
					   "Failed to create descriptor set layout");

	// create descriptor pool
	VkDescriptorPoolSize const texturePoolSize{
		.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		.descriptorCount = info.maxImageSamplers,
	};

	VkDescriptorPoolSize const uboPoolSize{
		.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		.descriptorCount = info.maxUniformBuffers,
	};

	VkDescriptorPoolSize const ssboPoolSize{
		.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		.descriptorCount = info.maxStorageBuffers,
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
		vkCreateDescriptorPool(info.device, &vk_descriptor_pool_create_info, nullptr,
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

	THROW_VULKAN_ERROR(
		vkAllocateDescriptorSets(info.device, &descriptorSetAllocateInfo,
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

	THROW_VULKAN_ERROR(vkCreatePipelineLayout(info.device, &pipelineLayoutInfo,
											  nullptr, m_pipelineLayouts.data()),
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

		THROW_VULKAN_ERROR(vkCreatePipelineLayout(info.device, &pipelineLayoutInfo,
												  nullptr, &m_pipelineLayouts.at(i)),
						   "Failed to create pipeline layout");
	}
}

void GpuResources::registerBuffer(VkBuffer buffer,
								  VkBufferUsageFlags usage,
								  VkDeviceSize size,
								  uint32_t binding) const {
	bool isStorageBuffer = (usage & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT) != 0;
	bool isUniformBuffer = (usage & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT) != 0;

	assert((isStorageBuffer || isUniformBuffer) && "Invalid buffer usage");

	VkDescriptorBufferInfo const bufferInfo{
		.buffer = buffer,
		.offset = 0,
		.range = size,
	};

	VkWriteDescriptorSet const writeDescriptorSet{
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = m_descriptorSet,
		.dstBinding = isStorageBuffer ? m_creationInfo.storageBuffersBinding
									  : m_creationInfo.uniformBuffersBinding,
		.dstArrayElement = binding,
		.descriptorCount = 1,
		.descriptorType = isStorageBuffer ? VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
										  : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		.pBufferInfo = &bufferInfo,
	};

	vkUpdateDescriptorSets(m_creationInfo.device, 1, &writeDescriptorSet, 0,
						   nullptr);
};

GpuResources::~GpuResources() {
	vkDestroyDescriptorSetLayout(m_creationInfo.device, m_descriptorSetLayout,
								 nullptr);
	vkDestroyDescriptorPool(m_creationInfo.device, m_descriptorPool, nullptr);

	for (auto layout : m_pipelineLayouts) {
		vkDestroyPipelineLayout(m_creationInfo.device, layout, nullptr);
	}
}
