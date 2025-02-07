#pragma once

#include <vulkan/vulkan_core.h>

namespace ignis {

class Device;
struct Queue;
class Pipeline;

template <typename T>
class Buffer;

template <typename T>
class Image;
class Sampler;

template <typename T>
class VertexBuffer;
class IndexBuffer;

class ColorImage;
class DepthImage;

struct DrawAttachment {
	ColorImage* drawImage;	// enforced to contain RENDER usage
	VkAttachmentLoadOp loadAction;
	VkAttachmentStoreOp storeAction;
};

struct DepthAttachment {
	DepthImage* depthImage;	 // enforced to contain DEPTH_STENCIL usage
	VkAttachmentLoadOp loadAction;
	VkAttachmentStoreOp storeAction;
};

// Note 1: for now every command is a graphics one
// Note 2: deallocation and resetting is per-command, not per-pool
// Note 3: samplers must be bound with a sampled image. We can't have samplers
// separate from sampled images
// Note 4: every draw command is indexed
// Note 5: for now we use default clear values
// Note 6: we'll use default values for the render area
// Note 7: we don't handle rendering to multiple draw attachments

class Command {
public:
	Command(Device, Queue);
	~Command();

	void begin(VkCommandBufferUsageFlags flags);

	void end();

	void bindPipeline(const Pipeline&);

	void beginRender(DrawAttachment, DepthAttachment);

	void beginRender(DepthAttachment);

	void endRendering();

	// will write sizeof(T) bytes starting at offset
	template <typename T>
	void pushConstants(const T& data, uint32_t offset = 0);

	template <typename T>
	void bindBuffer(const Buffer<T>&,
					uint32_t set,
					uint32_t binding,
					uint32_t arrayElement = 1);

	template <typename T>
	void bindSubBuffer(const Buffer<T>&,
					   uint32_t firstElement,
					   uint32_t lastElement,
					   uint32_t set,
					   uint32_t binding,
					   uint32_t arrayElement = 1);

	template <typename T>
	void bindStorageImage(const Image<T>&,
						  uint32_t set,
						  uint32_t binding,
						  uint32_t arrayElement = 1);

	template <typename T>
	void bindSampledImage(const Image<T>&,
						  const Sampler&,
						  uint32_t set,
						  uint32_t binding,
						  uint32_t arrayElement = 1);

	template <typename T>
	void bindVertexbuffer(const VertexBuffer<T>&);

	void bindIndexBuffer(const IndexBuffer&);

	void draw(uint32_t vertexCount, uint32_t firstVertex = 0);

	void drawInstanced(uint32_t vertexCount,
					   uint32_t instanceCount,
					   uint32_t firstVertex = 0,
					   uint32_t firstInstance = 0);

private:
	Device& m_device;
	VkQueue m_queue;
	VkCommandPool m_pool;
	VkCommandBuffer m_commandBuffer;

public:
	Command(const Command&) = delete;
	Command(Command&&) = delete;
	Command& operator=(const Command&) = delete;
	Command& operator=(Command&&) = delete;
};

}  // namespace ignis
