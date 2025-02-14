#include "buffer.hpp"
#include <cstring>

#include "device.hpp"
#include "exceptions.hpp"

using namespace ignis;

Buffer::Buffer(CreateInfo info)
	: m_device(*info.device),
	  m_stride(info.elementSize),
	  m_size(info.elementCount * info.elementSize) {
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
	size = size == 0 ? m_size : size;
	void* mappedData = nullptr;

	vmaMapMemory(m_device.getAllocator(), m_allocation, &mappedData);
	memcpy(static_cast<char*>(mappedData) + offset, data, size);
	vmaUnmapMemory(m_device.getAllocator(), m_allocation);
}
