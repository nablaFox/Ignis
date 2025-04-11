#include "ignis/fence.hpp"
#include "exceptions.hpp"

using namespace ignis;

Fence::Fence(const VkDevice device, bool signaled) : m_device(device) {
	VkFenceCreateInfo fenceInfo{
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
	};

	if (signaled) {
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	}

	THROW_VULKAN_ERROR(vkCreateFence(m_device, &fenceInfo, nullptr, &m_fence),
					   "Failed to create fence");
}

Fence::~Fence() {
	vkDestroyFence(m_device, m_fence, nullptr);
}

void Fence::wait() const {
	THROW_VULKAN_ERROR(vkWaitForFences(m_device, 1, &m_fence, VK_TRUE, UINT64_MAX),
					   "Failed to wait for fence");
}

void Fence::reset() const {
	THROW_VULKAN_ERROR(vkResetFences(m_device, 1, &m_fence),
					   "Failed to reset fence");
}

void Fence::waitAndReset() const {
	wait();
	reset();
}
