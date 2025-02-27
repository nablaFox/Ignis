#include "command.hpp"
#include "buffer.hpp"
#include "color_image.hpp"
#include "depth_image.hpp"
#include "device.hpp"
#include "image.hpp"
#include "sampler.hpp"
#include "shader.hpp"
#include "vk_utils.hpp"

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

	uint32_t srcAvailableWidth =
		src.m_extent.width - static_cast<uint32_t>(srcOffset.x);
	uint32_t srcAvailableHeight =
		src.m_extent.height - static_cast<uint32_t>(srcOffset.y);
	uint32_t dstAvailableWidth =
		dst.m_extent.width - static_cast<uint32_t>(dstOffset.x);
	uint32_t dstAvailableHeight =
		dst.m_extent.height - static_cast<uint32_t>(dstOffset.y);

	uint32_t regionWidth = std::min(srcAvailableWidth, dstAvailableWidth);
	uint32_t regionHeight = std::min(srcAvailableHeight, dstAvailableHeight);

	VkOffset3D srcStart{srcOffset.x, srcOffset.y, 0};
	VkOffset3D srcEnd{srcOffset.x + static_cast<int32_t>(regionWidth),
					  srcOffset.y + static_cast<int32_t>(regionHeight), 1};

	VkOffset3D dstStart{dstOffset.x, dstOffset.y, 0};
	VkOffset3D dstEnd{dstOffset.x + static_cast<int32_t>(regionWidth),
					  dstOffset.y + static_cast<int32_t>(regionHeight), 1};

	VkImageBlit2 blitRegion{
		.sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2,
		.pNext = nullptr,
		.srcSubresource =
			{
				.aspectMask = src.m_aspect,
				.mipLevel = 0,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
		.srcOffsets = {srcStart, srcEnd},
		.dstSubresource =
			{
				.aspectMask = dst.m_aspect,
				.mipLevel = 0,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
		.dstOffsets = {dstStart, dstEnd},
	};

	VkBlitImageInfo2 blitInfo{
		.sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2,
		.pNext = nullptr,
		.srcImage = src.m_handle,
		.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		.dstImage = dst.m_handle,
		.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		.regionCount = 1,
		.pRegions = &blitRegion,
		.filter = VK_FILTER_LINEAR,
	};

	vkCmdBlitImage2(m_commandBuffer, &blitInfo);
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

	Buffer* staging = Buffer::createStagingBuffer(
		&m_device, image.getPixelSize() * pixelsCount, pixels);

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

void Command::resolveImage(const ImageData& src, const ImageData& dst) {
	CHECK_IS_RECORDING;

	assert(src.m_currentLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL &&
		   "Source image is not in the correct layout");

	assert(dst.m_currentLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
		   "Destination image is not in the correct layout");

	VkImageResolve resolveRegion{
		.srcSubresource = {src.m_aspect, 0, 0, 1},
		.srcOffset = {0, 0, 0},
		.dstSubresource = {dst.m_aspect, 0, 0, 1},
		.dstOffset = {0, 0, 0},
		.extent = {src.m_extent.width, src.m_extent.height, 1},
	};

	vkCmdResolveImage(m_commandBuffer, src.m_handle,
					  VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst.m_handle,
					  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &resolveRegion);
}

void Command::updateBuffer(const Buffer& buffer,
						   const void* data,
						   uint32_t offset,
						   uint32_t size) {
	CHECK_IS_RECORDING;

	if (!size) {
		size = buffer.getSize() - offset;
	}

	THROW_ERROR(offset + size > buffer.getSize(), "Out of bounds");

	Buffer* staging = Buffer::createStagingBuffer(&m_device, size, data);

	m_stagingBuffers.push_back(staging);

	VkBufferCopy copyRegion = {
		.srcOffset = 0,
		.dstOffset = offset,
		.size = size,
	};

	vkCmdCopyBuffer(m_commandBuffer, staging->getHandle(), buffer.getHandle(), 1,
					&copyRegion);
}

void Command::bindPipeline(const Pipeline& pipeline) {
	CHECK_IS_RECORDING;

	VkDescriptorSet descriptorSet = m_device.getDescriptorSet();

	vkCmdBindDescriptorSets(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
							pipeline.getLayoutHandle(), 0, 1, &descriptorSet, 0,
							nullptr);

	vkCmdBindPipeline(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
					  pipeline.getHandle());

	m_currentPipeline = &pipeline;
}

void Command::beginRender(const DrawAttachment* drawAttachment,
						  const DepthAttachment* depthAttachment) {
	CHECK_IS_RECORDING;

	assert(drawAttachment != nullptr ||
		   depthAttachment != nullptr && "Both attachments are nullptr");

	VkRenderingAttachmentInfo colorAttachment{
		.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
		.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		.clearValue = {.color = drawAttachment->clearColor},
	};

	if (drawAttachment != nullptr) {
		THROW_ERROR((drawAttachment->drawImage->getUsage() &
					 VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) == 0,
					"Draw image must have COLOR_ATTACHMENT usage");

		assert(drawAttachment->drawImage->getCurrentLayout() ==
			   VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

		colorAttachment.imageView = drawAttachment->drawImage->getViewHandle();
		colorAttachment.loadOp = drawAttachment->loadAction;
		colorAttachment.storeOp = drawAttachment->storeAction;
	}

	VkRenderingAttachmentInfo depthAttachmentInfo{
		.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
		.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		.clearValue = {.depthStencil = {1.0f, 0}},
	};

	if (depthAttachment != nullptr) {
		THROW_ERROR((depthAttachment->depthImage->getUsage() &
					 VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) == 0,
					"Depth image must have DEPTH_STENCIL_ATTACHMENT usage");

		assert(depthAttachment->depthImage->getCurrentLayout() ==
			   VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

		depthAttachmentInfo.imageView = depthAttachment->depthImage->getViewHandle();
		depthAttachmentInfo.loadOp = depthAttachment->loadAction;
		depthAttachmentInfo.storeOp = depthAttachment->storeAction;
	}

	VkExtent2D extent = drawAttachment ? drawAttachment->drawImage->getExtent()
									   : depthAttachment->depthImage->getExtent();

	VkRenderingInfo renderingInfo{
		.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
		.renderArea = {{0, 0}, extent},
		.layerCount = 1,
		.colorAttachmentCount = 1,
		.pColorAttachments = drawAttachment ? &colorAttachment : nullptr,
		.pDepthAttachment = depthAttachment ? &depthAttachmentInfo : nullptr,
	};

	vkCmdBeginRendering(m_commandBuffer, &renderingInfo);
}

void Command::endRendering() {
	CHECK_IS_RECORDING;

	vkCmdEndRendering(m_commandBuffer);
}

void Command::setViewport(VkViewport viewport) {
	CHECK_IS_RECORDING;
	CHECK_PIPELINE_BOUND;

	vkCmdSetViewport(m_commandBuffer, 0, 1, &viewport);
}

void Command::setScissor(VkRect2D scissor) {
	CHECK_IS_RECORDING;
	CHECK_PIPELINE_BOUND;

	vkCmdSetScissor(m_commandBuffer, 0, 1, &scissor);
}

void Command::bindIndexBuffer(const Buffer& indexBuffer, VkDeviceSize offset) {
	CHECK_IS_RECORDING;
	CHECK_PIPELINE_BOUND;

	assert((indexBuffer.getUsage() & VK_BUFFER_USAGE_INDEX_BUFFER_BIT) != 0 &&
		   "Buffer is not an index buffer");

	VkIndexType indexType = indexBuffer.getSize() % sizeof(uint32_t) == 0
								? VK_INDEX_TYPE_UINT32
								: VK_INDEX_TYPE_UINT16;

	vkCmdBindIndexBuffer(m_commandBuffer, indexBuffer.getHandle(), offset,
						 indexType);
}

void Command::draw(uint32_t indexCount, uint32_t firstVertex) {
	CHECK_IS_RECORDING;
	CHECK_PIPELINE_BOUND;

	vkCmdDrawIndexed(m_commandBuffer, indexCount, 1, firstVertex, 0, 0);
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
