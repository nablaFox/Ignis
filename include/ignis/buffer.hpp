#pragma once

#include <vk_mem_alloc.h>
#include <span>

namespace ignis {

// Note 1: for now we don't provide an explicit memory usage flag
// instead we rely on VMA to automatically choose it for us

class Device;

class Buffer {
public:
	struct CreateInfo {
		Device* device;
		VkBufferUsageFlagBits bufferUsage;
		uint32_t elementCount;
		VkDeviceSize stride;
		std::span<void*> initialData;
	};

	Buffer(CreateInfo);

	virtual ~Buffer();

	VkDeviceSize getStride() const { return m_stride; }

	VkBuffer getHandle() const { return buffer; }

	// this only works for host visible memory
	void writeData(std::span<void*> data, uint32_t offset = 0);

	static Buffer createStagingBuffer(uint32_t elementCount, VkDeviceSize stride);

	static Buffer createIndexBuffer32(uint32_t elementCount,
									  std::span<uint32_t> indices = {});

private:
	Device& m_device;
	VkBufferUsageFlagBits bufferUsage;
	VmaAllocation allocation;
	VmaAllocationInfo allocationInfo;
	VkDeviceSize m_stride;
	uint32_t m_elementsCount;
	uint32_t size;
	VkDeviceAddress m_deviceAddress;
	VkBuffer buffer;

public:
	Buffer(const Buffer&) = delete;
	Buffer(Buffer&&) = delete;
	Buffer& operator=(const Buffer&) = delete;
	Buffer& operator=(Buffer&&) = delete;
};

}  // namespace ignis
