#include "command.hpp"
#include "buffer.hpp"
#include "color_image.hpp"
#include "depth_image.hpp"
#include "device.hpp"
#include "exceptions.hpp"
#include "image.hpp"
#include "pipeline.hpp"
#include "pipeline_layout.hpp"
#include "shader.hpp"
#include "vk_utils.hpp"

#include <cassert>

#define CHECK_IS_RECORDING \
	assert(m_isRecording && "Command buffer is not recording!");

#define CHECK_PIPELINE_BOUND \
	THROW_ERROR(m_currentPipeline == nullptr, "No pipeline bound");

using namespace ignis;

static void clearStagingBuffers(std::vector<Buffer*>& buffers) {
	for (auto& buffer : buffers) {
		delete buffer;
	}

	buffers.clear();
}

Command::Command(const Device& device, uint32_t queueIndex)
	: m_device(device), m_queueIndex(queueIndex) {
	THROW_ERROR(!m_device.getCommandPool(queueIndex, &m_commandPool),
				"Failed to get the command pool");

	VkCommandBufferAllocateInfo allocInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = m_commandPool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1,
	};

	THROW_VULKAN_ERROR(
		vkAllocateCommandBuffers(m_device.getDevice(), &allocInfo, &m_commandBuffer),
		"Failed to allocate command buffer");
}

Command::~Command() {
	clearStagingBuffers(m_stagingBuffers);
	vkFreeCommandBuffers(m_device.getDevice(), m_commandPool, 1, &m_commandBuffer);
}

void Command::begin(VkCommandBufferUsageFlags flags) {
	clearStagingBuffers(m_stagingBuffers);

	assert(!m_isRecording);

	VkCommandBufferBeginInfo beginInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = flags,
	};

	THROW_VULKAN_ERROR(vkBeginCommandBuffer(m_commandBuffer, &beginInfo),
					   "Failed to begin recording command");

	m_isRecording = true;
}

void Command::end() {
	CHECK_IS_RECORDING;

	THROW_VULKAN_ERROR(vkEndCommandBuffer(m_commandBuffer),
					   "Failed to end recording command");

	m_isRecording = false;
}

void Command::transitionImageLayout(Image& image, VkImageLayout newLayout) {
	CHECK_IS_RECORDING;

	VkImageMemoryBarrier barrier{
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.oldLayout = image.m_currentLayout,
		.newLayout = newLayout,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = image.m_image,
		.subresourceRange = {image.m_viewAspect, 0, 1, 0, 1},
	};

	TransitionInfo info = getTransitionInfo(image.m_currentLayout, newLayout);
	barrier.srcAccessMask = info.srcAccessMask;
	barrier.dstAccessMask = info.dstAccessMask;

	vkCmdPipelineBarrier(m_commandBuffer, info.srcStage, info.dstStage, 0, 0,
						 nullptr, 0, nullptr, 1, &barrier);

	image.m_currentLayout = newLayout;
}

void Command::updateImage(Image& image,
						  const void* pixels,
						  VkOffset2D imageOffset,
						  VkExtent2D imageSize) {
	CHECK_IS_RECORDING;

	VkExtent2D size =
		(!imageSize.width && !imageSize.height) ? image.m_extent : imageSize;

	uint32_t pixelsCount = size.width * size.height;

	Buffer* staging = Buffer::createStagingBuffer(&m_device, image.m_pixelSize,
												  pixelsCount, pixels);

	m_stagingBuffers.push_back(staging);

	staging->writeData(pixels);

	transitionImageLayout(image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	VkBufferImageCopy copyRegion = {
		.bufferOffset = 0,
		.imageSubresource = {image.m_viewAspect, 0, 0, 1},
		.imageOffset = {imageOffset.x, imageOffset.y, 0},
		.imageExtent = {size.width, size.height, 1},
	};

	vkCmdCopyBufferToImage(m_commandBuffer, staging->getHandle(), image.m_image,
						   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

	transitionImageLayout(image, image.getOptimalLayout());
}

void Command::updateBuffer(const Buffer& buffer,
						   const void* data,
						   uint32_t firstElement,
						   uint32_t lastElement) {
	CHECK_IS_RECORDING;

	if (lastElement == 0) {
		lastElement = buffer.getElementsCount();
	}

	uint32_t elementCount = lastElement - firstElement;
	uint32_t maxElementCount = buffer.getSize() / buffer.getStride();

	THROW_ERROR(elementCount > maxElementCount,
				"Buffer::updateBuffer: out of bounds");

	Buffer* staging = Buffer::createStagingBuffer(
		&m_device, buffer.getElementSize(), elementCount, data, buffer.getStride());

	m_stagingBuffers.push_back(staging);

	const VkDeviceSize dstOffset = firstElement * buffer.getStride();

	VkBufferCopy copyRegion = {
		.srcOffset = 0,
		.dstOffset = dstOffset,
		.size = staging->getSize(),
	};

	vkCmdCopyBuffer(m_commandBuffer, staging->getHandle(), buffer.getHandle(), 1,
					&copyRegion);
}

void Command::bindPipeline(const Pipeline& pipeline) {
	CHECK_IS_RECORDING;

	vkCmdBindPipeline(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
					  pipeline.getPipeline());

	m_currentPipeline = &pipeline;
}

static void bindBuffer(const Device& device,
					   VkCommandBuffer commandBuffer,
					   const Buffer& buffer,
					   uint32_t set,
					   uint32_t binding,
					   uint32_t arrayElement,
					   const PipelineLayout& pipelineLayout,
					   uint32_t firstElement = 0,
					   uint32_t lastElement = 0) {
	if (lastElement == 0) {
		lastElement = buffer.getElementsCount();
	}

	THROW_ERROR(lastElement > buffer.getElementsCount(),
				"Buffer::bindBuffer: out of bounds");

	VkDeviceSize range = buffer.getStride() * (lastElement - firstElement);
	VkDeviceSize offset = buffer.getStride() * firstElement;

	VkDescriptorBufferInfo bufferInfo{
		.buffer = buffer.getHandle(),
		.offset = offset,
		.range = range,
	};

	const BindingInfo& bindingInfo = pipelineLayout.getBindingInfo(set, binding);

	VkWriteDescriptorSet descriptorWrite{
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = VK_NULL_HANDLE,
		.dstBinding = binding,
		.dstArrayElement = arrayElement,
		.descriptorCount = bindingInfo.arraySize,
		.descriptorType = bindingInfo.bindingType,
		.pBufferInfo = &bufferInfo,
	};

	device.getPushDescriptorFunc()(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
								   pipelineLayout.getHandle(), set, 1,
								   &descriptorWrite);
}

void Command::bindUBO(const Buffer& buffer,
					  uint32_t set,
					  uint32_t binding,
					  uint32_t arrayElement) {
	CHECK_IS_RECORDING;
	CHECK_PIPELINE_BOUND;

	assert(buffer.getUsage() == VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT &&
		   "Buffer is not a UBO");

	bindBuffer(m_device, m_commandBuffer, buffer, set, binding, arrayElement,
			   m_currentPipeline->getLayout());
}

void Command::bindSSBO(const Buffer& buffer,
					   uint32_t set,
					   uint32_t binding,
					   uint32_t arrayElement) {
	CHECK_IS_RECORDING;
	CHECK_PIPELINE_BOUND;

	assert(buffer.getUsage() == VK_BUFFER_USAGE_STORAGE_BUFFER_BIT &&
		   "Buffer is not a SSBO");

	bindBuffer(m_device, m_commandBuffer, buffer, set, binding, arrayElement,
			   m_currentPipeline->getLayout());
}

void Command::bindSubSSBO(const Buffer& buffer,
						  uint32_t firstElement,
						  uint32_t lastElement,
						  uint32_t set,
						  uint32_t binding,
						  uint32_t arrayElement) {
	CHECK_IS_RECORDING;
	CHECK_PIPELINE_BOUND;

	assert(buffer.getUsage() == VK_BUFFER_USAGE_STORAGE_BUFFER_BIT &&
		   "bindSubSSBO: Buffer is not a SSBO");

	bindBuffer(m_device, m_commandBuffer, buffer, set, binding, arrayElement,
			   m_currentPipeline->getLayout(), firstElement, lastElement);
}

void Command::bindSubUBO(const Buffer& buffer,
						 uint32_t firstElement,
						 uint32_t lastElement,
						 uint32_t set,
						 uint32_t binding,
						 uint32_t arrayElement) {
	CHECK_IS_RECORDING;
	CHECK_PIPELINE_BOUND;

	assert(buffer.getUsage() == VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT &&
		   "bindSubUBO: Buffer is not a UBO");

	bindBuffer(m_device, m_commandBuffer, buffer, set, binding, arrayElement,
			   m_currentPipeline->getLayout(), firstElement, lastElement);
}

void Command::bindIndexBuffer(const Buffer& indexBuffer, VkDeviceSize offset) {
	CHECK_IS_RECORDING;

	assert(indexBuffer.getUsage() == VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
	assert(indexBuffer.getStride() == sizeof(uint32_t) ||
		   indexBuffer.getStride() == sizeof(uint16_t));

	VkIndexType indexType = indexBuffer.getStride() == sizeof(uint32_t)
								? VK_INDEX_TYPE_UINT32
								: VK_INDEX_TYPE_UINT16;

	vkCmdBindIndexBuffer(m_commandBuffer, indexBuffer.getHandle(), offset,
						 indexType);
}

void Command::beginRender(DrawAttachment drawAttachment,
						  DepthAttachment depthAttachment) {
	CHECK_IS_RECORDING;

	THROW_ERROR(
		drawAttachment.drawImage->getUsage() != VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		"Draw image must have COLOR_ATTACHMENT usage");

	THROW_ERROR(depthAttachment.depthImage->getUsage() !=
					VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
				"Depth image must have DEPTH_STENCIL_ATTACHMENT usage");

	assert(drawAttachment.drawImage->getCurrentLayout() ==
		   VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	VkRenderingAttachmentInfo colorAttachment{
		.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
		.imageView = drawAttachment.drawImage->getView(),
		.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		.loadOp = drawAttachment.loadAction,
		.storeOp = drawAttachment.storeAction,
		.clearValue = {.color = {0.0f, 0.0f, 0.0f, 1.0f}},
	};

	assert(depthAttachment.depthImage->getCurrentLayout() ==
		   VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

	VkRenderingAttachmentInfo depthAttachmentInfo{
		.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
		.imageView = depthAttachment.depthImage->getView(),
		.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		.loadOp = depthAttachment.loadAction,
		.storeOp = depthAttachment.storeAction,
		.clearValue = {.depthStencil = {1.0f, 0}},
	};

	VkExtent2D extent = drawAttachment.drawImage->getExtent();

	VkRenderingInfo renderingInfo{
		.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
		.renderArea = {{0, 0}, extent},
		.layerCount = 1,
		.colorAttachmentCount = 1,
		.pColorAttachments = &colorAttachment,
		.pDepthAttachment = &depthAttachmentInfo,
	};

	vkCmdBeginRendering(m_commandBuffer, &renderingInfo);
}

void Command::beginRender(DepthAttachment depthAttachment) {
	CHECK_IS_RECORDING;

	THROW_ERROR(depthAttachment.depthImage->getUsage() !=
					VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
				"Depth image in depth attachment has not the correct usage");

	assert(depthAttachment.depthImage->getCurrentLayout() ==
		   VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

	VkRenderingAttachmentInfo depthAttachmentInfo{
		.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
		.imageView = depthAttachment.depthImage->getView(),
		.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		.loadOp = depthAttachment.loadAction,
		.storeOp = depthAttachment.storeAction,
		.clearValue = {.depthStencil = {1.0f, 0}},
	};

	VkExtent2D extent = depthAttachment.depthImage->getExtent();

	VkRenderingInfo renderingInfo{
		.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
		.renderArea = {{0, 0}, extent},
		.layerCount = 1,
		.colorAttachmentCount = 0,
		.pColorAttachments = nullptr,
		.pDepthAttachment = &depthAttachmentInfo,
	};

	vkCmdBeginRendering(m_commandBuffer, &renderingInfo);
}

void Command::endRendering() {
	CHECK_IS_RECORDING;

	vkCmdEndRendering(m_commandBuffer);
}

void Command::draw(uint32_t vertexCount, uint32_t firstVertex) {
	CHECK_IS_RECORDING;
	CHECK_PIPELINE_BOUND;

	vkCmdDrawIndexed(m_commandBuffer, vertexCount, 1, firstVertex, 0, 0);
}

void Command::drawInstanced(uint32_t vertexCount,
							uint32_t instanceCount,
							uint32_t firstVertex,
							uint32_t firstInstance) {
	CHECK_IS_RECORDING;
	CHECK_PIPELINE_BOUND;

	vkCmdDrawIndexed(m_commandBuffer, vertexCount, instanceCount, firstVertex, 0,
					 firstInstance);
}
