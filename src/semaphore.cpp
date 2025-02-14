#include "semaphore.hpp"
#include "device.hpp"
#include "exceptions.hpp"

using namespace ignis;

Semaphore::Semaphore(const Device& device) : m_device(device) {
	VkSemaphoreCreateInfo semaphoreInfo{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
	};

	THROW_VULKAN_ERROR(vkCreateSemaphore(m_device.getDevice(), &semaphoreInfo,
										 nullptr, &m_semaphore),
					   "Failed to create semaphore");
}

Semaphore::~Semaphore() {
	vkDestroySemaphore(m_device.getDevice(), m_semaphore, nullptr);
}
