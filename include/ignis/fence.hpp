#pragma once

#include <vulkan/vulkan_core.h>

namespace ignis {

class Device;

class Fence {
public:
	Fence(const Device& device);
	~Fence();

	VkFence getHandle() const { return m_fence; }

	void wait() const;

private:
	const Device& m_device;
	VkFence m_fence{nullptr};

public:
	Fence(const Fence&) = delete;
	Fence(Fence&&) = delete;
	Fence& operator=(const Fence&) = delete;
	Fence& operator=(Fence&&) = delete;
};

}  // namespace ignis
