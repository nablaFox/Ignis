#pragma once

#include <vulkan/vulkan_core.h>

// Note 1: we can wait for a single fence
// Note 2: we can reset a single fence at a time

namespace ignis {

class Fence {
public:
	Fence(const VkDevice, bool signaled = false);
	~Fence();

	VkFence getHandle() const { return m_fence; }

	void reset() const;

	void wait() const;

	void waitAndReset() const;

private:
	const VkDevice m_device;
	VkFence m_fence{nullptr};

public:
	Fence(const Fence&) = delete;
	Fence(Fence&&) = delete;
	Fence& operator=(const Fence&) = delete;
	Fence& operator=(Fence&&) = delete;
};

}  // namespace ignis
