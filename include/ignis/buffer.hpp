#pragma once

#include <vk_mem_alloc.h>
#include "device.hpp"

// Note 1: updates are relative to whole elements; if a buffer is created with the
// type T we can't update partially a single element T

namespace ignis {

class Device;

class Buffer {
public:
	struct CreateInfo {
		const Device* device;
		VkBufferUsageFlags bufferUsage;
		VkMemoryPropertyFlags memoryProperties;
		VkDeviceSize elementSize;
		uint32_t elementCount;
		VkDeviceSize stride;
		const void* initialData;
	};

	Buffer(CreateInfo);

	~Buffer();

	VkDeviceSize getStride() const { return m_stride; }

	VkBuffer getHandle() const { return m_buffer; }

	void writeData(const void* data,
				   VkDeviceSize firstElement = 0,
				   VkDeviceSize lastElement = 0);

	template <typename T>
	static Buffer* createStagingBuffer(const Device* device,
									   uint32_t elementCount,
									   const T* data = nullptr);

	// - stride = sizeof(uint32_t)
	// - elementCount = span.size() / sizeof(uint32_t)
	// - usage = INDEX_BUFFER_BIT
	static Buffer* createIndexBuffer32(const Device*,
									   uint32_t elementCount,
									   uint32_t* data = nullptr);

	template <typename T>
	static Buffer* createUBO(const Device* device,
							 uint32_t elementCount,
							 const T* data = nullptr) {
		VkDeviceSize alignment = device->getUboAlignment();
		VkDeviceSize stride = (sizeof(T) + alignment - 1) & ~(alignment - 1);

		Buffer::CreateInfo info{
			.device = device,
			.bufferUsage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			.memoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
								VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			.elementSize = sizeof(T),
			.elementCount = elementCount,
			.stride = stride,
			.initialData = data,
		};

		return new Buffer(info);
	}

	// TODO add vertex buffer creation
	template <typename T>
	static Buffer* createVertexBuffer(const Device*,
									  uint32_t elementCount,
									  const T* data = nullptr);

	// TODO add storage buffer creation

	VkDeviceAddress getDeviceAddress() const { return m_deviceAddress; }

private:
	const Device& m_device;
	VmaAllocation m_allocation{nullptr};
	VkDeviceSize m_stride;
	VkDeviceSize m_size;
	VkDeviceSize m_elementSize;
	VkDeviceAddress m_deviceAddress{0};
	VkBuffer m_buffer{nullptr};
	VkMemoryPropertyFlags m_memoryProperties;

public:
	Buffer(const Buffer&) = delete;
	Buffer(Buffer&&) = delete;
	Buffer& operator=(const Buffer&) = delete;
	Buffer& operator=(Buffer&&) = delete;
};

}  // namespace ignis
