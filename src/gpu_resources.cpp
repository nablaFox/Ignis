#include <cassert>
#include <vector>
#include "ignis/gpu_resources.hpp"
#include "ignis/buffer.hpp"
#include "ignis/image.hpp"
#include "ignis/exceptions.hpp"

using namespace ignis;

GpuResources::GpuResources(const BindlessResourcesCreateInfo& info)
	: m_creationInfo(info), m_device(info.device) {
	assert(m_device && "Invalid device");
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

	THROW_VULKAN_ERROR(vkCreateDescriptorSetLayout(m_device, &createInfo, nullptr,
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
		vkCreateDescriptorPool(m_device, &vk_descriptor_pool_create_info, nullptr,
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

	THROW_VULKAN_ERROR(vkAllocateDescriptorSets(m_device, &descriptorSetAllocateInfo,
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

	THROW_VULKAN_ERROR(vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, nullptr,
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

		THROW_VULKAN_ERROR(vkCreatePipelineLayout(m_device, &pipelineLayoutInfo,
												  nullptr, &m_pipelineLayouts.at(i)),
						   "Failed to create pipeline layout");
	}
}

GpuResources::~GpuResources() {
	vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayout, nullptr);
	vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr);

	for (auto layout : m_pipelineLayouts) {
		vkDestroyPipelineLayout(m_device, layout, nullptr);
	}
}

// TODO: recycle buffer ids
BufferId GpuResources::registerBuffer(Buffer buffer) {
	const bool isStorageBuffer =
		(buffer.getUsage() & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT) != 0;
	const bool isUniformBuffer =
		(buffer.getUsage() & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT) != 0;

	assert((isStorageBuffer || isUniformBuffer) && "Invalid buffer usage");

	BufferId id = nextBufferId++;

	VkDescriptorBufferInfo const bufferInfo{
		.buffer = buffer.getHandle(),
		.offset = 0,
		.range = buffer.getSize(),
	};

	VkWriteDescriptorSet const writeDescriptorSet{
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = m_descriptorSet,
		.dstBinding = isStorageBuffer ? m_creationInfo.storageBuffersBinding
									  : m_creationInfo.uniformBuffersBinding,
		.dstArrayElement = id,
		.descriptorCount = 1,
		.descriptorType = isStorageBuffer ? VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
										  : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		.pBufferInfo = &bufferInfo,
	};

	vkUpdateDescriptorSets(m_device, 1, &writeDescriptorSet, 0, nullptr);

	m_buffers.insert({id, std::make_unique<Buffer>(std::move(buffer))});

	return id;
};

// TODO: recycle image ids
BufferId GpuResources::registerImage(Image image) {
	const bool isStorageImage = (image.getUsage() & VK_IMAGE_USAGE_STORAGE_BIT) != 0;
	const bool isSampledImage = (image.getUsage() & VK_IMAGE_USAGE_SAMPLED_BIT) != 0;

	assert((isStorageImage || isSampledImage) && "Invalid image usage");

	ImageId id = nextImageId++;

	// TODO: implement

	return id;
}

VkPipelineLayout GpuResources::getPipelinelayout(
	VkDeviceSize pushConstantSize) const {
	THROW_ERROR(
		static_cast<uint32_t>(pushConstantSize) > 4 * MAX_PUSH_CONSTANT_WORD_SIZE,
		"Invalid push constant size");

	return m_pipelineLayouts.at(1 + (pushConstantSize / 4));
}

Buffer& GpuResources::getBuffer(BufferId id) const {
	auto it = m_buffers.find(id);

	THROW_ERROR(it == m_buffers.end(), "Invalid buffer handle");

	return *it->second;
}

Image& GpuResources::getImage(ImageId id) const {
	auto it = m_images.find(id);

	THROW_ERROR(it == m_images.end(), "Invalid image handle");

	return *it->second;
}

void GpuResources::destroyBuffer(BufferId& id) {
	auto it = m_buffers.find(id);

	THROW_ERROR(it == m_buffers.end(), "Invalid buffer handle");

	m_buffers.erase(it);

	id = IGNIS_INVALID_BUFFER_ID;
}

void GpuResources::destroyImage(ImageId& id) {
	auto it = m_images.find(id);

	THROW_ERROR(it == m_images.end(), "Invalid image handle");

	m_images.erase(it);

	id = IGNIS_INVALID_IMAGE_ID;
}
