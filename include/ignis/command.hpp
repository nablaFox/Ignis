#pragma once

#include <vulkan/vulkan_core.h>
#include <cassert>
#include <memory>
#include <vector>
#include "pipeline.hpp"

namespace ignis {

class Device;
class Pipeline;
class Buffer;
class Image;

struct DrawAttachment {
	Image* drawImage{nullptr};
	VkAttachmentLoadOp loadAction{VK_ATTACHMENT_LOAD_OP_CLEAR};
	VkAttachmentStoreOp storeAction{VK_ATTACHMENT_STORE_OP_STORE};
	VkClearColorValue clearColor{0.0f, 0.0f, 0.0f, 1.0f};
};

struct DepthAttachment {
	Image* depthImage{nullptr};
	VkAttachmentLoadOp loadAction{VK_ATTACHMENT_LOAD_OP_CLEAR};
	VkAttachmentStoreOp storeAction{VK_ATTACHMENT_STORE_OP_DONT_CARE};
};

struct CommandCreateInfo {
	Device& device;
	VkQueue queue;
};

#define CHECK_IS_RECORDING \
	assert(m_isRecording && "Command buffer is not recording!");

#define CHECK_PIPELINE_BOUND assert(m_pipelineBound && "Pipeline is not bound!");

typedef uint32_t BufferId;

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
	Command(const CommandCreateInfo&);
	~Command();

	void begin(VkCommandBufferUsageFlags flags =
				   VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	void end();

	void bindPipeline(const Pipeline&);

	void beginRender(const DrawAttachment*, const DepthAttachment*);

	void endRendering();

	template <typename T>
	void pushConstants(const Pipeline& pipeline,
					   const T& data,
					   uint32_t offset = 0) {
		CHECK_IS_RECORDING;
		CHECK_PIPELINE_BOUND;

		auto pipelineLayout = pipeline.getLayoutHandle();

		vkCmdPushConstants(m_commandBuffer, pipelineLayout, VK_SHADER_STAGE_ALL,
						   offset, sizeof(T), &data);
	}

	void transitionImageLayout(Image&, VkImageLayout);
	void transitionToOptimalLayout(Image&);

	void copyImage(const Image& src,
				   const Image& dst,
				   VkOffset2D srcOffset = {0, 0},
				   VkOffset2D dstOffset = {0, 0});

	void blitImage(const Image& src,
				   const Image& dst,
				   VkOffset2D srcOffset = {0, 0},
				   VkOffset2D dstOffset = {0, 0});

	void resolveImage(const Image& src, const Image& dst);

	void updateImage(const Image&,
					 const void* pixels,
					 VkOffset2D imageOffset = {0, 0},
					 VkExtent2D imageSize = {0, 0});

	void updateBuffer(BufferId,
					  const void* data,
					  uint32_t offset = 0,
					  uint32_t size = 0);

	void setViewport(VkViewport);
	void setScissor(VkRect2D);

	void bindIndexBuffer(BufferId, VkDeviceSize offset = 0);

	void draw(uint32_t indexCount, uint32_t firstIndex = 0);

	void drawInstanced(uint32_t vertexCount,
					   uint32_t instanceCount,
					   uint32_t firstVertex = 0,
					   uint32_t firstInstance = 0);

	VkQueue getQueue() const { return m_queue; }

	VkCommandBuffer getHandle() const { return m_commandBuffer; }

private:
	Device& m_device;
	VkQueue m_queue;
	VkCommandPool m_commandPool{nullptr};

	VkCommandBuffer m_commandBuffer{nullptr};
	bool m_isRecording{false};
	bool m_pipelineBound{false};
	std::vector<std::unique_ptr<Buffer>> m_stagingBuffers;

public:
	Command(const Command&) = delete;
	Command(Command&&) = delete;
	Command& operator=(const Command&) = delete;
	Command& operator=(Command&&) = delete;
};

}  // namespace ignis
