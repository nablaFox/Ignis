#include "buffer.hpp"
#include <cassert>
#include <cstring>
#include "exceptions.hpp"
#include "device.hpp"

using namespace ignis;

Buffer::Buffer(CreateInfo info)
	: m_device(*info.device),
	  m_memoryProperties(info.memoryProperties),
	  m_bufferUsage(info.bufferUsage),
	  m_size(info.size) {
	assert(m_size > 0 && "Buffer size must be greater than 0");

	VkBufferCreateInfo bufferInfo{
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = m_size,
		.usage = info.bufferUsage | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
	};

	VmaAllocationCreateInfo allocationInfo{
		.requiredFlags = info.memoryProperties,
	};

	THROW_VULKAN_ERROR(
		vmaCreateBuffer(m_device.getAllocator(), &bufferInfo, &allocationInfo,
						&m_buffer, &m_allocation, nullptr),
		"Failed to create buffer");

	VkBufferDeviceAddressInfo addressInfo{
		.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
		.buffer = m_buffer,
	};

	m_deviceAddress = vkGetBufferDeviceAddress(m_device.getDevice(), &addressInfo);

	if (info.initialData) {
		writeData(info.initialData);
	}
}

Buffer::~Buffer() {
	vmaDestroyBuffer(m_device.getAllocator(), m_buffer, m_allocation);
}

void Buffer::writeData(const void* data, uint32_t offset, uint32_t size) {
	THROW_ERROR(!(m_memoryProperties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT),
				"Writing to non-host visible buffer");

	if (!size) {
		size = m_size - offset;
	}

	THROW_ERROR(offset + size > m_size, "Out of bounds");

	void* mappedData = nullptr;
	vmaMapMemory(m_device.getAllocator(), m_allocation, &mappedData);

	char* dst = static_cast<char*>(mappedData) + offset;
	memcpy(dst, data, size);

	if (!(m_memoryProperties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
		vmaFlushAllocation(m_device.getAllocator(), m_allocation, offset, size);
	}

	vmaUnmapMemory(m_device.getAllocator(), m_allocation);
}

void Buffer::readData(void* data, uint32_t offset, uint32_t size) {
	THROW_ERROR(!(m_memoryProperties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT),
				"Reading from non-host visible buffer");

	if (!size) {
		size = m_size - offset;
	}

	THROW_ERROR(offset + size > size, "Out of bounds");

	void* mappedData = nullptr;
	vmaMapMemory(m_device.getAllocator(), m_allocation, &mappedData);

	char* src = static_cast<char*>(mappedData) + offset;
	memcpy(data, src, size);

	vmaUnmapMemory(m_device.getAllocator(), m_allocation);

	if (!(m_memoryProperties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
		vmaInvalidateAllocation(m_device.getAllocator(), m_allocation, offset, size);
	}
}

Buffer Buffer::createUBO(const Device* device, uint32_t size, const void* data) {
	VkDeviceSize alignment = device->getUboAlignment();
	VkDeviceSize bufferSize = (size + alignment - 1) & ~(alignment - 1);

	return Buffer({
		.device = device,
		.bufferUsage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		.memoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
							VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		.size = bufferSize,
		.initialData = data,
	});
}

Buffer Buffer::createSSBO(const Device* device, uint32_t size, const void* data) {
	VkDeviceSize alignment = device->getSsboAlignment();
	VkDeviceSize bufferSize = (size + alignment - 1) & ~(alignment - 1);

	return Buffer({
		.device = device,
		.bufferUsage =
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		.memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		.size = bufferSize,
		.initialData = data,
	});
}

Buffer Buffer::createIndexBuffer32(const Device* device,
								   uint32_t elementCount,
								   uint32_t* data) {
	return Buffer({
		.device = device,
		.bufferUsage =
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		.memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		.size = sizeof(uint32_t) * elementCount,
		.initialData = data,
	});
}

Buffer Buffer::createStagingBuffer(const Device* device,
								   VkDeviceSize size,
								   const void* data) {
	return Buffer({
		.device = device,
		.bufferUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		.memoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
							VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		.size = size,
		.initialData = data,
	});
}
