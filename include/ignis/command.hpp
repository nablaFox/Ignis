#pragma once

#include <vulkan/vulkan_core.h>
#include <cassert>
#include <vector>
#include "exceptions.hpp"
#include "pipeline.hpp"

namespace ignis {

class Device;
class Pipeline;
class Buffer;
class Image;
struct ImageData;
class ColorImage;
class DepthImage;

struct DrawAttachment {
	ColorImage* drawImage{nullptr};
	VkAttachmentLoadOp loadAction{VK_ATTACHMENT_LOAD_OP_CLEAR};
	VkAttachmentStoreOp storeAction{VK_ATTACHMENT_STORE_OP_STORE};
	VkClearColorValue clearColor{0.0f, 0.0f, 0.0f, 1.0f};
};

struct DepthAttachment {
	DepthImage* depthImage{nullptr};
	VkAttachmentLoadOp loadAction{VK_ATTACHMENT_LOAD_OP_CLEAR};
	VkAttachmentStoreOp storeAction{VK_ATTACHMENT_STORE_OP_DONT_CARE};
};

#define CHECK_IS_RECORDING \
	assert(m_isRecording && "Command buffer is not recording!");

#define CHECK_PIPELINE_BOUND \
	THROW_ERROR(m_currentPipeline == nullptr, "No pipeline bound");

// Note 1: every command is a graphics command
// Note 2: every command is primary
// Note 3: allocation, deallocation and resetting is per-command, not per-pool, i.e.
// we can't batch those operations for multiple commands
// Note 4: every draw command is indexed
// Note 5: clear values are fixed
// Note 6: the render area is fixed
// Note 7: we can only render to 1 draw attachment

class Command {
public:
	Command(const Device&, uint32_t queueIndex = 0);
	~Command();

	void begin(VkCommandBufferUsageFlags flags =
				   VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	void end();

	void bindPipeline(const Pipeline&);

	void beginRender(const DrawAttachment*, const DepthAttachment*);

	void beginRender(DepthAttachment);

	void endRendering();

	template <typename T>
	void pushConstants(const T& data, uint32_t offset = 0) {
		CHECK_IS_RECORDING;
		CHECK_PIPELINE_BOUND;

		auto pipelineLayout = m_currentPipeline->getLayoutHandle();

		vkCmdPushConstants(m_commandBuffer, pipelineLayout, VK_SHADER_STAGE_ALL,
						   offset, sizeof(T), &data);
	}

	void transitionImageLayout(ImageData&, VkImageLayout);
	void transitionToOptimalLayout(ImageData&);

	void copyImage(const ImageData& src,
				   const ImageData& dst,
				   VkOffset2D srcOffset = {0, 0},
				   VkOffset2D dstOffset = {0, 0});

	void blitImage(const ImageData& src,
				   const ImageData& dst,
				   VkOffset2D srcOffset = {0, 0},
				   VkOffset2D dstOffset = {0, 0});

	void resolveImage(const ImageData& src, const ImageData& dst);

	void updateImage(const Image&,
					 const void* pixels,
					 VkOffset2D imageOffset = {0, 0},
					 VkExtent2D imageSize = {0, 0});

	void updateBuffer(const Buffer&,
					  const void* data,
					  uint32_t offset = 0,
					  uint32_t size = 0);

	void setViewport(VkViewport);
	void setScissor(VkRect2D);

	void bindIndexBuffer(const Buffer&, VkDeviceSize offset = 0);

	void draw(uint32_t indexCount, uint32_t firstIndex = 0);

	void drawInstanced(uint32_t vertexCount,
					   uint32_t instanceCount,
					   uint32_t firstVertex = 0,
					   uint32_t firstInstance = 0);

	uint32_t getQueueIndex() const { return m_queueIndex; }

	VkCommandBuffer getHandle() const { return m_commandBuffer; }

private:
	const Device& m_device;
	VkCommandPool m_commandPool{nullptr};
	uint32_t m_queueIndex;
	VkCommandBuffer m_commandBuffer{nullptr};
	bool m_isRecording{false};
	std::vector<Buffer*> m_stagingBuffers;
	const Pipeline* m_currentPipeline{nullptr};

public:
	Command(const Command&) = delete;
	Command(Command&&) = delete;
	Command& operator=(const Command&) = delete;
	Command& operator=(Command&&) = delete;
};

}  // namespace ignis
