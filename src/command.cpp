#include "command.hpp"
#include "buffer.hpp"
#include "device.hpp"
#include "exceptions.hpp"
#include "image.hpp"
#include "pipeline.hpp"
#include "vk_utils.hpp"

using namespace ignis;

static void clearStagingBuffers(std::vector<Buffer*>& buffers) {
	for (auto& buffer : buffers) {
		delete buffer;
	}

	buffers.clear();
}

Command::Command(const Device& device, uint32_t queueIndex)
	: m_device(device), m_queueIndex(queueIndex) {
	THROW_ERROR(m_device.getCommandPool(queueIndex, &m_commandPool),
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

	// TODO add warning if the command buffer is already recording

	VkCommandBufferBeginInfo beginInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = flags,
	};

	THROW_VULKAN_ERROR(vkBeginCommandBuffer(m_commandBuffer, &beginInfo),
					   "Failed to begin recording command");

	m_isRecording = true;
}

void Command::end() {
	// TODO add warning if the command buffer is not recording

	THROW_VULKAN_ERROR(vkEndCommandBuffer(m_commandBuffer),
					   "Failed to end recording command");

	m_isRecording = false;
}

void Command::transitionImageLayout(Image& image, VkImageLayout newLayout) {
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

// the user must provide the correct data type for pixels
// as to match image format
void Command::updateImage(Image& image,
						  const void* pixels,
						  VkOffset2D imageOffset,
						  VkExtent2D imageSize) {
	VkExtent2D size =
		(!imageSize.width && !imageSize.height) ? image.m_extent : imageSize;

	uint32_t pixelsCount = size.width * size.height;

	Buffer* staging = new Buffer({
		.device = &m_device,
		.bufferUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		.memoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
							VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		.elementCount = pixelsCount,
		.elementSize = image.m_pixelSize,
		.initialData = nullptr,
	});

	m_stagingBuffers.push_back(staging);

	staging->writeData(pixels, pixelsCount * image.m_pixelSize);

	transitionImageLayout(image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	VkBufferImageCopy copyRegion = {
		.bufferOffset = 0,
		.imageSubresource = {image.m_viewAspect, 0, 0, 1},
		.imageOffset = {imageOffset.x, imageOffset.y, 0},
		.imageExtent = {size.width, size.height, 1},
	};

	vkCmdCopyBufferToImage(m_commandBuffer, staging->getHandle(), image.m_image,
						   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);
}
