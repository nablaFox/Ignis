#pragma once

#include <vulkan/vulkan_core.h>
#include <vector>

namespace ignis {

class Device;
class Pipeline;
class Buffer;
class Image;
struct ImageData;
class Sampler;
class ColorImage;
class DepthImage;

struct DrawAttachment {
	ColorImage* drawImage;
	VkAttachmentLoadOp loadAction;
	VkAttachmentStoreOp storeAction;
};

struct DepthAttachment {
	DepthImage* depthImage;
	VkAttachmentLoadOp loadAction;
	VkAttachmentStoreOp storeAction;
};

// Note 1: every command is a graphics command
// Note 2: every command is primary
// Note 3: allocation, deallocation and resetting is per-command, not per-pool, i.e.
// we can't batch those operations for multiple commands
// Note 4: we can't bind single samplers
// Note 5: every draw command is indexed
// Note 6: clear values are fixed
// Note 7: the render area is fixed
// Note 8: we can only render to 1 draw attachment
// Note 9: we can only bind 1 buffer/image at a time
// Note 10: we rely on push descriptors for binding buffers and images

class Command {
public:
	Command(const Device&, uint32_t queueIndex = 0);
	~Command();

	void begin(VkCommandBufferUsageFlags flags =
				   VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	void end();

	void bindPipeline(const Pipeline&);

	void beginRender(DrawAttachment, DepthAttachment);

	void beginRender(DepthAttachment);

	void endRendering();

	template <typename T>
	void pushConstants(const T& data, uint32_t offset = 0);

	void transitionImageLayout(ImageData&, VkImageLayout);
	void transitionToOptimalLayout(ImageData&);

	void copyImage(const ImageData& src,
				   const ImageData& dst,
				   VkOffset2D srcOffset = {0, 0},
				   VkOffset2D dstOffset = {0, 0});

	void updateImage(const Image&,
					 const void* pixels,
					 VkOffset2D imageOffset = {0, 0},
					 VkExtent2D imageSize = {0, 0});

	void updateBuffer(const Buffer&,
					  const void* data,
					  uint32_t firstElement = 0,
					  uint32_t lastElement = 0);

	void bindUBO(const Buffer&,
				 uint32_t set,
				 uint32_t binding,
				 uint32_t arrayElement = 0);

	void bindSSBO(const Buffer&,
				  uint32_t set,
				  uint32_t binding,
				  uint32_t arrayElement = 0);

	void bindSubSSBO(const Buffer&,
					 uint32_t firstElement,
					 uint32_t lastElement,
					 uint32_t set,
					 uint32_t binding,
					 uint32_t arrayElement = 0);

	void bindSubUBO(const Buffer&,
					uint32_t firstElement,
					uint32_t lastElement,
					uint32_t set,
					uint32_t binding,
					uint32_t arrayElement = 0);

	void bindStorageImage(const Image&,
						  uint32_t set,
						  uint32_t binding,
						  uint32_t arrayElement = 0);

	void bindSampledImage(const Image&,
						  const Sampler&,
						  uint32_t set,
						  uint32_t binding,
						  uint32_t arrayElement = 0);

	void bindIndexBuffer(const Buffer&, VkDeviceSize offset = 0);

	void draw(uint32_t vertexCount, uint32_t firstVertex = 0);

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
