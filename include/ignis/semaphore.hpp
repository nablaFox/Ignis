#pragma once

#include <vulkan/vulkan_core.h>

namespace ignis {

class Device;

class Semaphore {
public:
	Semaphore(const Device& device);
	~Semaphore();

	VkSemaphore getHandle() const { return m_semaphore; }

	void wait() const;

private:
	const Device& m_device;
	VkSemaphore m_semaphore{nullptr};

public:
	Semaphore(const Semaphore&) = delete;
	Semaphore(Semaphore&&) = delete;
	Semaphore& operator=(const Semaphore&) = delete;
	Semaphore& operator=(Semaphore&&) = delete;
};

}  // namespace ignis
