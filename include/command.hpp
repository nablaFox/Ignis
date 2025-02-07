#pragma once

#include <vulkan/vulkan_core.h>

namespace ignis {

class Device;
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

// Note 1: every command is a graphics one
// Note 2: every command is a primary one
// Note 3: allocation, deallocation and resetting is per-command, not per-pool, i.e.
// we can't batch those operations for multiple commands
// Note 4: we can't bind single samplers
// Note 5: every draw command is indexed
// Note 6: clear values are fixed
// Note 7: the render area is fixed
// Note 8: we can only render to 1 draw attachment
// Note 9: we can only bind 1 buffer/image at a time

class Command {
public:
	Command(Device, VkQueue);
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
	void bindVertexbuffer(const VertexBuffer<T>&, uint32_t binding);

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
