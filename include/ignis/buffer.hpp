#pragma once

#include <vulkan/vulkan_core.h>

struct VmaAllocator_T;
struct VmaAllocation_T;

namespace ignis {

struct BufferCreateInfo {
	VkBufferUsageFlags bufferUsage;
	VkMemoryPropertyFlags memoryProperties;
	VkDeviceSize size;
	const void* initialData;
};

class Buffer {
public:
	Buffer(VmaAllocator_T*, const BufferCreateInfo&);

	~Buffer();

	auto getSize() const { return m_size; }

	auto getHandle() const { return m_buffer; }

	auto getUsage() const { return m_bufferUsage; }

public:
	void writeData(const void* data, VkDeviceSize offset = 0, uint32_t size = 0);

	void readData(void* data, VkDeviceSize offset = 0, uint32_t size = 0);

	VkDeviceAddress getDeviceAddress(VkDevice) const;

	static Buffer allocateUBO(VmaAllocator_T*,
							  VkDeviceSize alignment,
							  VkDeviceSize size,
							  const void* data = nullptr);

	static Buffer allocateSSBO(VmaAllocator_T*,
							   VkDeviceSize alignment,
							   VkDeviceSize size,
							   const void* data = nullptr);

	static Buffer allocateIndexBuffer32(VmaAllocator_T*,
										uint32_t elementCount,
										const uint32_t* data = nullptr);

	static Buffer allocateStagingBuffer(VmaAllocator_T*,
										VkDeviceSize size,
										const void* data = nullptr);

private:
	VmaAllocator_T* m_allocator{nullptr};
	VmaAllocation_T* m_allocation{nullptr};
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
