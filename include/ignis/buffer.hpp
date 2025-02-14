#pragma once

#include <vk_mem_alloc.h>

namespace ignis {

class Device;

class Buffer {
public:
	struct CreateInfo {
		const Device* device;
		VkBufferUsageFlags bufferUsage;
		VkMemoryPropertyFlags memoryProperties;
		uint32_t elementCount;
		VkDeviceSize elementSize;
		const void* initialData;
	};

	Buffer(CreateInfo);

	~Buffer();

	VkDeviceSize getStride() const { return m_stride; }

	VkBuffer getHandle() const { return m_buffer; }

	// this only works for host visible memory
	void writeData(const void* data, uint32_t offset = 0, uint32_t size = 0);

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

	// - stride = (elementSize + alignment - 1) & ~(alignment - 1);
	// - usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT
	template <typename T>
	static Buffer* createUBO(const Device*,
							 uint32_t elementCount,
							 const T* data = nullptr);

	// TODO add vertex buffer creation
	// TODO add storage buffer creation

private:
	const Device& m_device;
	VmaAllocation m_allocation{nullptr};
	VkDeviceSize m_stride;
	VkDeviceSize m_size;
	VkDeviceAddress m_deviceAddress{0};
	VkBuffer m_buffer{nullptr};

public:
	Buffer(const Buffer&) = delete;
	Buffer(Buffer&&) = delete;
	Buffer& operator=(const Buffer&) = delete;
	Buffer& operator=(Buffer&&) = delete;
};

}  // namespace ignis
