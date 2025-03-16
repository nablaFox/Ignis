#pragma once

#include <vk_mem_alloc.h>

namespace ignis {

struct BufferCreateInfo {
	VkBufferUsageFlags bufferUsage;
	VkMemoryPropertyFlags memoryProperties;
	VkDeviceSize size;
	const void* initialData;
};

class Buffer {
public:
	Buffer(VmaAllocator, const BufferCreateInfo&);

	~Buffer();

	VkDeviceSize getSize() const { return m_size; }

	VkBuffer getHandle() const { return m_buffer; }

	VkBufferUsageFlags getUsage() const { return m_bufferUsage; }

	VkDeviceAddress getDeviceAddress(VkDevice);

	void writeData(const void* data, uint32_t offset = 0, uint32_t size = 0);

	void readData(void* data, uint32_t offset = 0, uint32_t size = 0);

	static BufferCreateInfo uboDesc(VkDeviceSize alignment,
									VkDeviceSize size,
									const void* data = nullptr);

	static BufferCreateInfo ssboDesc(VkDeviceSize alignment,
									 VkDeviceSize size,
									 const void* data = nullptr);

	static BufferCreateInfo indexBuffer32Desc(uint32_t elementCount,
											  uint32_t* data = nullptr);

	static BufferCreateInfo stagingBufferDesc(VkDeviceSize size,
											  const void* data = nullptr);

private:
	VmaAllocator m_allocator{nullptr};
	VmaAllocation m_allocation{nullptr};
	VkDeviceSize m_size;
	VkDeviceAddress m_deviceAddress{0};
	VkBufferUsageFlags m_bufferUsage;
	VkBuffer m_buffer{nullptr};
	VkMemoryPropertyFlags m_memoryProperties;

public:
	Buffer(Buffer&& other) noexcept;
	Buffer& operator=(Buffer&& other) = delete;
	Buffer(const Buffer&) = delete;
	Buffer& operator=(const Buffer&) = delete;
};

}  // namespace ignis
