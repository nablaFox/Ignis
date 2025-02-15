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
	  m_memoryProperties(info.memoryProperties),
	  m_bufferUsage(info.bufferUsage) {
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

void Buffer::writeData(const void* data,
					   uint32_t firstElement,
					   uint32_t lastElement) {
	THROW_ERROR(!(m_memoryProperties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT),
				"Writing to non-host visible buffer");

	if (lastElement == 0) {
		lastElement = m_size - firstElement;
	}

	THROW_ERROR(lastElement % m_elementSize != 0,
				"Size must be a multiple of element size");

	VkDeviceSize requiredSpace = (getElementsCount() - 1) * m_stride + m_elementSize;

	THROW_ERROR(firstElement + requiredSpace > m_size, "Out of bounds");

	void* mappedData;
	vmaMapMemory(m_device.getAllocator(), m_allocation, &mappedData);

	const uint8_t* src = static_cast<const uint8_t*>(data);
	uint8_t* dst = static_cast<uint8_t*>(mappedData) + firstElement;

	for (VkDeviceSize i = 0; i < getElementsCount(); ++i) {
		std::memcpy(dst + i * m_stride, src + i * m_elementSize, m_elementSize);
	}

	vmaUnmapMemory(m_device.getAllocator(), m_allocation);

	if (!(m_memoryProperties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
		vmaFlushAllocation(m_device.getAllocator(), m_allocation, firstElement,
						   requiredSpace);
	}
}

Buffer* Buffer::createStagingBuffer(const Device* device,
									VkDeviceSize elementSize,
									uint32_t elementCount,
									const void* data,
									VkDeviceSize stride) {
	return new Buffer({
		.device = device,
		.bufferUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		.memoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
							VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		.elementSize = elementSize,
		.elementCount = elementCount,
		.stride = stride,
		.initialData = data,
	});
}

Buffer* Buffer::createIndexBuffer32(const Device* device,
									uint32_t elementCount,
									uint32_t* data) {
	return new Buffer({
		.device = device,
		.bufferUsage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		.memoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
							VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		.elementSize = sizeof(uint32_t),
		.elementCount = elementCount,
		.stride = sizeof(uint32_t),
		.initialData = data,
	});
}
