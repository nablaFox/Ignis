#include "buffer.hpp"
#include <cstring>
#include "exceptions.hpp"
#include "device.hpp"

using namespace ignis;

Buffer::Buffer(CreateInfo info)
	: m_device(*info.device),
	  m_stride(info.stride ? info.stride : info.elementSize),
	  m_size(info.elementCount * m_stride),
	  m_elementSize(info.elementSize),
	  m_memoryProperties(info.memoryProperties) {
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

void Buffer::writeData(const void* data, VkDeviceSize offset, VkDeviceSize size) {
	THROW_ERROR(!(m_memoryProperties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT),
				"Writing to non-host visible buffer");

	if (size == 0) {
		size = m_size - offset;
	}

	THROW_ERROR(size % m_elementSize != 0,
				"Buffer::writeData: size must be a multiple of element size");

	VkDeviceSize elementCount = size / m_stride;
	VkDeviceSize requiredSpace = (elementCount - 1) * m_stride + m_elementSize;

	THROW_ERROR(offset + requiredSpace > m_size, "Buffer::writeData: out of bounds");

	void* mappedData;
	vmaMapMemory(m_device.getAllocator(), m_allocation, &mappedData);

	const uint8_t* src = static_cast<const uint8_t*>(data);
	uint8_t* dst = static_cast<uint8_t*>(mappedData) + offset;

	for (VkDeviceSize i = 0; i < elementCount; ++i) {
		std::memcpy(dst + i * m_stride, src + i * m_elementSize, m_elementSize);
	}

	vmaUnmapMemory(m_device.getAllocator(), m_allocation);

	if (!(m_memoryProperties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
		vmaFlushAllocation(m_device.getAllocator(), m_allocation, offset,
						   requiredSpace);
	}
}
