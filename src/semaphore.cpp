#include "semaphore.hpp"
#include "device.hpp"
#include "exceptions.hpp"

using namespace ignis;

Semaphore::Semaphore(VkDevice device) : m_device(device) {
	assert(device != nullptr && "Invalid device");

	VkSemaphoreCreateInfo semaphoreInfo{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
	};

	THROW_VULKAN_ERROR(
		vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_semaphore),
		"Failed to create semaphore");
}

Semaphore::~Semaphore() {
	vkDestroySemaphore(m_device, m_semaphore, nullptr);
}
