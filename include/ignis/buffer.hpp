#pragma once

#include <vk_mem_alloc.h>

namespace ignis {

class Buffer {
	// these are the classess that can allocate buffers
	friend class Device;
	friend class Command;

public:
	// public struct for users
	struct CreateInfo {
		VkBufferUsageFlags bufferUsage;
		VkMemoryPropertyFlags memoryProperties;
		VkDeviceSize size;
		const void* initialData;
	};

	Buffer(VkDevice,
		   VkBuffer,
		   VkDeviceAddress,
		   VmaAllocation,
		   VmaAllocator,
		   const CreateInfo&);

	~Buffer();

	VkDeviceSize getSize() const { return m_size; }

	VkBuffer getHandle() const { return m_buffer; }

	VkBufferUsageFlags getUsage() const { return m_bufferUsage; }

	VkDeviceAddress getDeviceAddress() const { return m_bufferAddress; }

	void writeData(const void* data, uint32_t offset = 0, uint32_t size = 0);

	void readData(void* data, uint32_t offset = 0, uint32_t size = 0);

private:
	VkDevice m_device;
	VkBuffer m_buffer;
	VkDeviceAddress m_bufferAddress;
	VmaAllocation m_allocation;
	VmaAllocator m_allocator;

	VkDeviceSize m_size;
	VkBufferUsageFlags m_bufferUsage;
	VkMemoryPropertyFlags m_memoryProperties;

	static Buffer allocateBuffer(VkDevice, VmaAllocator, const CreateInfo&);

	static Buffer allocateUBO(VkDevice,
							  VkDeviceSize alignment,
							  VmaAllocator,
							  VkDeviceSize,
							  const void* initialData);

	static Buffer allocateSSBO(VkDevice,
							   VkDeviceSize alignment,
							   VmaAllocator,
							   VkDeviceSize,
							   const void* initialData);

	static Buffer allocateStagingBuffer(VkDevice,
										VmaAllocator,
										VkDeviceSize,
										const void* initialData);

	static Buffer allocateIndexBuffer32(VkDevice,
										VmaAllocator,
										uint32_t indicesCount,
										uint32_t* data);

public:
	Buffer(const Buffer&) = delete;
	Buffer& operator=(const Buffer&) = delete;
	Buffer(Buffer&&) noexcept = default;
	Buffer& operator=(Buffer&&) noexcept = delete;
};

}  // namespace ignis
