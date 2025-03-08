#include <cassert>
#include <cstring>
#include "exceptions.hpp"
#include "buffer.hpp"

using namespace ignis;

Buffer::Buffer(VkDevice device, VmaAllocator allocator, const CreateInfo& info)
	: m_device(device), m_allocator(allocator), m_creationInfo(info) {
	assert(m_device != nullptr && "Invalid device");
	assert(m_allocator != nullptr && "Invalid allocator");
	assert(info.size > 0 && "Invalid buffer size");

	VkBufferCreateInfo bufferInfo{
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = info.size,
		.usage = info.bufferUsage | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
	};

	VmaAllocationCreateInfo allocationInfo{
		.requiredFlags = info.memoryProperties,
	};

	THROW_VULKAN_ERROR(vmaCreateBuffer(allocator, &bufferInfo, &allocationInfo,
									   &m_buffer, &m_allocation, nullptr),
					   "Failed to create buffer");

	VkBufferDeviceAddressInfo addressInfo{
		.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
		.buffer = m_buffer,
	};

	m_bufferAddress = vkGetBufferDeviceAddress(device, &addressInfo);

	if (info.initialData) {
		writeData(info.initialData);
	}
}

Buffer::~Buffer() {
	vmaDestroyBuffer(m_allocator, m_buffer, m_allocation);
}

void Buffer::writeData(const void* data, uint32_t offset, uint32_t size) {
	THROW_ERROR(
		!(m_creationInfo.memoryProperties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT),
		"Writing to non-host visible buffer");

	if (!size) {
		size = m_creationInfo.size - offset;
	}

	THROW_ERROR(offset + size > m_creationInfo.size, "Out of bounds");

	void* mappedData = nullptr;
	vmaMapMemory(m_allocator, m_allocation, &mappedData);

	char* dst = static_cast<char*>(mappedData) + offset;
	memcpy(dst, data, size);

	if (!(m_creationInfo.memoryProperties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
		vmaFlushAllocation(m_allocator, m_allocation, offset, size);
	}

	vmaUnmapMemory(m_allocator, m_allocation);
}

void Buffer::readData(void* data, uint32_t offset, uint32_t size) {
	THROW_ERROR(
		!(m_creationInfo.memoryProperties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT),
		"Reading from non-host visible buffer");

	if (!size) {
		size = m_creationInfo.size - offset;
	}

	THROW_ERROR(offset + size > size, "Out of bounds");

	void* mappedData = nullptr;
	vmaMapMemory(m_allocator, m_allocation, &mappedData);

	char* src = static_cast<char*>(mappedData) + offset;
	memcpy(data, src, size);

	vmaUnmapMemory(m_allocator, m_allocation);

	if (!(m_creationInfo.memoryProperties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
		vmaInvalidateAllocation(m_allocator, m_allocation, offset, size);
	}
}

Buffer Buffer::allocateUBO(VkDevice device,
						   VkDeviceSize alignment,
						   VmaAllocator allocator,
						   VkDeviceSize size,
						   const void* data) {
	VkDeviceSize bufferSize = (size + alignment - 1) & ~(alignment - 1);

	Buffer::CreateInfo info{
		.bufferUsage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		.memoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
							VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		.size = bufferSize,
		.initialData = data,
	};

	return Buffer(device, allocator, info);
}

Buffer Buffer::allocateSSBO(VkDevice device,
							VkDeviceSize alignment,
							VmaAllocator allocator,
							VkDeviceSize size,
							const void* data) {
	VkDeviceSize bufferSize = (size + alignment - 1) & ~(alignment - 1);

	Buffer::CreateInfo info{
		.bufferUsage =
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		.memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		.size = bufferSize,
		.initialData = data,
	};

	return Buffer(device, allocator, info);
}

Buffer Buffer::allocateStagingBuffer(VkDevice device,
									 VmaAllocator allocator,
									 VkDeviceSize size,
									 const void* data) {
	Buffer::CreateInfo info{
		.bufferUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		.memoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
							VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		.size = size,
		.initialData = data,
	};

	return Buffer(device, allocator, info);
}

Buffer Buffer::allocateIndexBuffer32(VkDevice device,
									 VmaAllocator allocator,
									 uint32_t indicesCount,
									 uint32_t* data) {
	Buffer::CreateInfo info{
		.bufferUsage =
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		.memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		.size = sizeof(uint32_t) * indicesCount,
		.initialData = data,
	};

	return Buffer(device, allocator, info);
}
