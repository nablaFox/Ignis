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
		VkDeviceSize size;
		const void* initialData;
	};

	Buffer(CreateInfo);

	~Buffer();

	VkDeviceSize getSize() const { return m_size; }

	VkBuffer getHandle() const { return m_buffer; }

	VkBufferUsageFlags getUsage() const { return m_bufferUsage; }

	VkDeviceAddress getDeviceAddress() const { return m_deviceAddress; }

	void writeData(const void* data, uint32_t offset = 0, uint32_t size = 0);

	void readData(void* data, uint32_t offset = 0, uint32_t size = 0);

	static Buffer* createUBO(const Device* device,
							 uint32_t size,
							 const void* data = nullptr);

	static Buffer* createSSBO(const Device* device,
							  uint32_t size,
							  const void* data = nullptr);

	static Buffer* createVertexBuffer(const Device* device,
									  uint32_t size,
									  const void* data = nullptr);

	static Buffer* createIndexBuffer32(const Device*,
									   uint32_t elementCount,
									   uint32_t* data = nullptr);

	static Buffer* createStagingBuffer(const Device* device,
									   VkDeviceSize size,
									   const void* data = nullptr);

private:
	const Device& m_device;
	VmaAllocation m_allocation{nullptr};
	VkDeviceSize m_size;
	VkDeviceAddress m_deviceAddress{0};
	VkBufferUsageFlags m_bufferUsage;
	VkBuffer m_buffer{nullptr};
	VkMemoryPropertyFlags m_memoryProperties;

public:
	Buffer(const Buffer&) = delete;
	Buffer(Buffer&&) = delete;
	Buffer& operator=(const Buffer&) = delete;
	Buffer& operator=(Buffer&&) = delete;
};

}  // namespace ignis
