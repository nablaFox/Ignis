#include <cassert>
#include <cstring>
#include "buffer.hpp"
#include "exceptions.hpp"

using namespace ignis;

Buffer::Buffer(VmaAllocator allocator, const BufferCreateInfo& info)
	: m_allocator(allocator),
	  m_memoryProperties(info.memoryProperties),
	  m_bufferUsage(info.bufferUsage),
	  m_size(info.size) {
	assert(m_size > 0 && "Buffer size must be greater than 0");
	assert(m_allocator && "Invalid allocator");

	VkBufferCreateInfo const bufferInfo{
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = m_size,
		.usage = info.bufferUsage | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
	};

	VmaAllocationCreateInfo const allocationInfo{
		.requiredFlags = info.memoryProperties,
	};

	THROW_VULKAN_ERROR(vmaCreateBuffer(m_allocator, &bufferInfo, &allocationInfo,
									   &m_buffer, &m_allocation, nullptr),
					   "Failed to allocate buffer");

	if (info.initialData) {
		writeData(info.initialData);
	}
}

Buffer::~Buffer() {
	vmaDestroyBuffer(m_allocator, m_buffer, m_allocation);
}

Buffer::Buffer(Buffer&& other) noexcept
	: m_allocator(other.m_allocator),
	  m_memoryProperties(other.m_memoryProperties),
	  m_bufferUsage(other.m_bufferUsage),
	  m_size(other.m_size),
	  m_buffer(other.m_buffer),
	  m_allocation(other.m_allocation) {
	other.m_buffer = VK_NULL_HANDLE;
	other.m_allocation = VK_NULL_HANDLE;
}

void Buffer::writeData(const void* data, VkDeviceSize offset, uint32_t size) {
	THROW_ERROR(!(m_memoryProperties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT),
				"Writing to non-host visible buffer");

	if (!size) {
		size = m_size - offset;
	}

	THROW_ERROR(offset + size > m_size, "Out of bounds");

	void* mappedData = nullptr;

	vmaMapMemory(m_allocator, m_allocation, &mappedData);

	char* dst = static_cast<char*>(mappedData) + offset;
	memcpy(dst, data, size);

	if (!(m_memoryProperties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
		vmaFlushAllocation(m_allocator, m_allocation, offset, size);
	}

	vmaUnmapMemory(m_allocator, m_allocation);
}

void Buffer::readData(void* data, VkDeviceSize offset, uint32_t size) {
	THROW_ERROR(!(m_memoryProperties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT),
				"Reading from non-host visible buffer");

	if (!size) {
		size = m_size - offset;
	}

	THROW_ERROR(offset + size > size, "Out of bounds");

	void* mappedData = nullptr;
	vmaMapMemory(m_allocator, m_allocation, &mappedData);

	char* src = static_cast<char*>(mappedData) + offset;
	memcpy(data, src, size);

	vmaUnmapMemory(m_allocator, m_allocation);

	if (!(m_memoryProperties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
		vmaInvalidateAllocation(m_allocator, m_allocation, offset, size);
	}
}

VkDeviceAddress Buffer::getDeviceAddress(VkDevice device) const {
	VkBufferDeviceAddressInfo const addressInfo{
		.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
		.buffer = m_buffer,
	};

	return vkGetBufferDeviceAddress(device, &addressInfo);
}

Buffer Buffer::allocateUBO(VmaAllocator allocator,
						   VkDeviceSize alignment,
						   VkDeviceSize size,
						   const void* data) {
	VkDeviceSize bufferSize = (size + alignment - 1) & ~(alignment - 1);

	BufferCreateInfo info{
		.bufferUsage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		.memoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
							VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		.size = bufferSize,
		.initialData = data,
	};

	return Buffer(allocator, std::move(info));
}

Buffer Buffer::allocateSSBO(VmaAllocator allocator,
							VkDeviceSize alignment,
							VkDeviceSize size,
							const void* data) {
	VkDeviceSize bufferSize = (size + alignment - 1) & ~(alignment - 1);

	BufferCreateInfo info{
		.bufferUsage =
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		.memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		.size = bufferSize,
		.initialData = data,
	};

	return Buffer(allocator, std::move(info));
}

Buffer Buffer::allocateIndexBuffer32(VmaAllocator allocator,
									 uint32_t elementCount,
									 const uint32_t* data) {
	BufferCreateInfo info{
		.bufferUsage =
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		.memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		.size = sizeof(uint32_t) * elementCount,
		.initialData = data,
	};

	return Buffer(allocator, std::move(info));
}

Buffer Buffer::allocateStagingBuffer(VmaAllocator allocator,
									 VkDeviceSize size,
									 const void* data) {
	BufferCreateInfo info{
		.bufferUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		.memoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
							VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		.size = size,
		.initialData = data,
	};

	return Buffer(allocator, std::move(info));
}
