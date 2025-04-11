#include "ignis/semaphore.hpp"
#include "exceptions.hpp"

using namespace ignis;

Semaphore::Semaphore(const VkDevice device) : m_device(device) {
	VkSemaphoreCreateInfo const semaphoreInfo{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
	};

	THROW_VULKAN_ERROR(
		vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_semaphore),
		"Failed to create semaphore");
}

Semaphore::~Semaphore() {
	vkDestroySemaphore(m_device, m_semaphore, nullptr);
}
