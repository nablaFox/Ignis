#include "command.hpp"
#include "buffer.hpp"
#include "color_image.hpp"
#include "depth_image.hpp"
#include "device.hpp"
#include "exceptions.hpp"
#include "image.hpp"
#include "pipeline.hpp"
#include "pipeline_layout.hpp"
#include "sampler.hpp"
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
	: m_device(device),
	  m_commandPool(m_device.getCommandPool(queueIndex)),
	  m_queueIndex(queueIndex) {
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

void Command::transitionImageLayout(ImageData& image, VkImageLayout newLayout) {
	CHECK_IS_RECORDING;

	TransitionInfo transitionInfo =
		getTransitionInfo(image.m_currentLayout, newLayout);

	VkImageMemoryBarrier barrier{
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.srcAccessMask = transitionInfo.srcAccessMask,
		.dstAccessMask = transitionInfo.dstAccessMask,
		.oldLayout = image.m_currentLayout,
		.newLayout = newLayout,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = image.m_handle,
		.subresourceRange = {image.m_aspect, 0, 1, 0, 1},
	};

	vkCmdPipelineBarrier(m_commandBuffer, transitionInfo.srcStage,
						 transitionInfo.dstStage, 0, 0, nullptr, 0, nullptr, 1,
						 &barrier);

	image.m_currentLayout = newLayout;
}

void Command::transitionToOptimalLayout(ImageData& image) {
	CHECK_IS_RECORDING;

	transitionImageLayout(image, image.m_optimalLayout);
}

void Command::copyImage(const ImageData& src,
						const ImageData& dst,
						VkOffset2D srcOffset,
						VkOffset2D dstOffset) {
	assert(src.m_currentLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL &&
		   "Source image is not in the correct layout");

	assert(dst.m_currentLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
		   "Destination image is not in the correct layout");

	VkImageSubresourceLayers srcSubresource{
		.aspectMask = src.m_aspect,
		.mipLevel = 0,
		.baseArrayLayer = 0,
		.layerCount = 1,
	};

	VkImageSubresourceLayers dstSubresource{
		.aspectMask = dst.m_aspect,
		.mipLevel = 0,
		.baseArrayLayer = 0,
		.layerCount = 1,
	};

	VkImageCopy copyRegion{
		.srcSubresource = srcSubresource,
		.srcOffset = {srcOffset.x, srcOffset.y, 0},
		.dstSubresource = dstSubresource,
		.dstOffset = {dstOffset.x, dstOffset.y, 0},
		.extent = {src.m_extent.width, src.m_extent.height, 1},
	};

	vkCmdCopyImage(m_commandBuffer, src.m_handle,
				   VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst.m_handle,
				   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);
}

void Command::blitImage(const ImageData& src,
						const ImageData& dst,
						VkOffset2D srcOffset,
						VkOffset2D dstOffset) {
	assert(src.m_currentLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL &&
		   "Source image is not in the correct layout");
	assert(dst.m_currentLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
		   "Destination image is not in the correct layout");

	VkOffset3D srcOffset3D{srcOffset.x, srcOffset.y, 0};
	VkOffset3D dstOffset3D{dstOffset.x, dstOffset.y, 0};
	VkOffset3D extent3D{static_cast<int32_t>(src.m_extent.width),
						static_cast<int32_t>(src.m_extent.height), 1};
	VkOffset3D dstExtent3D{static_cast<int32_t>(dst.m_extent.width),
						   static_cast<int32_t>(dst.m_extent.height), 1};

	VkImageBlit blitRegion{
		.srcSubresource = {src.m_aspect, 0, 0, 1},
		.srcOffsets = {srcOffset3D, extent3D},
		.dstSubresource = {dst.m_aspect, 0, 0, 1},
		.dstOffsets = {dstOffset3D, dstExtent3D},
	};

	vkCmdBlitImage(m_commandBuffer, src.m_handle,
				   VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst.m_handle,
				   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blitRegion,
				   VK_FILTER_LINEAR);
}

void Command::updateImage(const Image& image,
						  const void* pixels,
						  VkOffset2D imageOffset,
						  VkExtent2D imageSize) {
	CHECK_IS_RECORDING;

	assert(image.getCurrentLayout() == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
		   "Image is not in the correct layout");

	VkExtent2D size =
		(!imageSize.width && !imageSize.height) ? image.getExtent() : imageSize;

	uint32_t pixelsCount = size.width * size.height;

	Buffer* staging = Buffer::createStagingBuffer(&m_device, image.getPixelSize(),
												  pixelsCount, pixels);

	m_stagingBuffers.push_back(staging);

	staging->writeData(pixels);

	VkBufferImageCopy copyRegion = {
		.bufferOffset = 0,
		.imageSubresource = {image.getAspect(), 0, 0, 1},
		.imageOffset = {imageOffset.x, imageOffset.y, 0},
		.imageExtent = {size.width, size.height, 1},
	};

	vkCmdCopyBufferToImage(m_commandBuffer, staging->getHandle(), image.getHandle(),
						   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);
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

void Command::bindSampledImage(const Image& image,
							   const Sampler& sampler,
							   uint32_t set,
							   uint32_t binding,
							   uint32_t arrayElement) {
	CHECK_IS_RECORDING;
	CHECK_PIPELINE_BOUND;

	assert(image.getUsage() == VK_IMAGE_USAGE_SAMPLED_BIT &&
		   "Image is not a sampled image");

	VkDescriptorImageInfo imageInfo{
		.sampler = sampler.getHandle(),
		.imageView = image.getViewHandle(),
		.imageLayout = image.getCurrentLayout(),
	};

	const BindingInfo& bindingInfo =
		m_currentPipeline->getLayout().getBindingInfo(set, binding);

	VkWriteDescriptorSet descriptorWrite{
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = VK_NULL_HANDLE,
		.dstBinding = binding,
		.dstArrayElement = arrayElement,
		.descriptorCount = bindingInfo.arraySize,
		.descriptorType = bindingInfo.bindingType,
		.pImageInfo = &imageInfo,
	};

	m_device.getPushDescriptorFunc()(
		m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
		m_currentPipeline->getLayout().getHandle(), set, 1, &descriptorWrite);
}

void Command::bindStorageImage(const Image& image,
							   uint32_t set,
							   uint32_t binding,
							   uint32_t arrayElement) {
	CHECK_IS_RECORDING;
	CHECK_PIPELINE_BOUND;

	assert(image.getUsage() == VK_IMAGE_USAGE_STORAGE_BIT &&
		   "Image is not a storage image");

	VkDescriptorImageInfo imageInfo{
		.imageView = image.getViewHandle(),
		.imageLayout = image.getCurrentLayout(),
	};

	const BindingInfo& bindingInfo =
		m_currentPipeline->getLayout().getBindingInfo(set, binding);

	VkWriteDescriptorSet descriptorWrite{
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = VK_NULL_HANDLE,
		.dstBinding = binding,
		.dstArrayElement = arrayElement,
		.descriptorCount = bindingInfo.arraySize,
		.descriptorType = bindingInfo.bindingType,
		.pImageInfo = &imageInfo,
	};

	m_device.getPushDescriptorFunc()(
		m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
		m_currentPipeline->getLayout().getHandle(), set, 1, &descriptorWrite);
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
		.imageView = drawAttachment.drawImage->getViewHandle(),
		.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		.loadOp = drawAttachment.loadAction,
		.storeOp = drawAttachment.storeAction,
		.clearValue = {.color = {0.0f, 0.0f, 0.0f, 1.0f}},
	};

	assert(depthAttachment.depthImage->getCurrentLayout() ==
		   VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

	VkRenderingAttachmentInfo depthAttachmentInfo{
		.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
		.imageView = depthAttachment.depthImage->getViewHandle(),
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
		.imageView = depthAttachment.depthImage->getViewHandle(),
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
