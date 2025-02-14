#include "device.hpp"
#include "exceptions.hpp"
#include "fence.hpp"

using namespace ignis;

Fence::Fence(const Device& device) : m_device(device) {
	VkFenceCreateInfo fenceInfo{
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.flags = VK_FENCE_CREATE_SIGNALED_BIT,
	};

	THROW_VULKAN_ERROR(
		vkCreateFence(m_device.getDevice(), &fenceInfo, nullptr, &m_fence),
		"Failed to create fence");
}

Fence::~Fence() {
	vkDestroyFence(m_device.getDevice(), m_fence, nullptr);
}

void Fence::wait() const {
	THROW_VULKAN_ERROR(
		vkWaitForFences(m_device.getDevice(), 1, &m_fence, VK_TRUE, UINT64_MAX),
		"Failed to wait for fence");
}
