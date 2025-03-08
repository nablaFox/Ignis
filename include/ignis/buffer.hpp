#pragma once

#include <vk_mem_alloc.h>

namespace ignis {

class Buffer {
public:
	struct CreateInfo {
		VkBufferUsageFlags bufferUsage{VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
									   VK_BUFFER_USAGE_TRANSFER_DST_BIT};
		VkMemoryPropertyFlags memoryProperties{VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
											   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};
		VkDeviceSize size{0};
		const void* initialData{nullptr};
	};

	// VmaAllocator should be created relative to the same VkDevice
	// passed here. This is not enforced but is required and will be documented
	Buffer(VkDevice,
		   VkBuffer,
		   VkDeviceAddress,
		   VmaAllocation,
		   VmaAllocator,
		   const CreateInfo&);

	~Buffer();

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

	VkDeviceSize getSize() const { return m_creationInfo.size; }

	VkBuffer getHandle() const { return m_buffer; }

	VkBufferUsageFlags getUsage() const { return m_creationInfo.bufferUsage; }

	VkDeviceAddress getDeviceAddress() const { return m_bufferAddress; }

	void writeData(const void* data, uint32_t offset = 0, uint32_t size = 0);

	void readData(void* data, uint32_t offset = 0, uint32_t size = 0);

private:
	VkDevice m_device;
	VkBuffer m_buffer;
	VkDeviceAddress m_bufferAddress;
	VmaAllocation m_allocation;
	VmaAllocator m_allocator;

	CreateInfo m_creationInfo;

public:
	Buffer(const Buffer&) = delete;
	Buffer& operator=(const Buffer&) = delete;
	Buffer(Buffer&&) = default;
	Buffer& operator=(Buffer&&) = delete;
};

}  // namespace ignis
