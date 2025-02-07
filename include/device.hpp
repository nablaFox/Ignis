#pragma once

#include <vulkan/vulkan_core.h>
#include <vector>

namespace ignis {

class Semaphore;
class Fence;
class Command;

// Note 1: the library supports just 1 instance, physical and logical device
// Note 2: we handle only graphics queues

class Device {
public:
	Device();
	~Device();

	bool getQueue(uint32_t index, VkQueue*) const;

	void submitCommands(const std::vector<Command>&,
						const std::vector<Semaphore>& waitSemaphores,
						const std::vector<Semaphore>& signalSemaphores,
						const Fence&);

	VkCommandPool getCommandPool();

	VkDevice getDevice() const { return m_device; }

private:
	std::vector<VkCommandPool> m_pools;
	std::vector<std::vector<VkQueue>> m_queues;
	VkDevice m_device;

public:
	Device(const Device&) = default;
	Device(Device&&) = delete;
	Device& operator=(const Device&) = default;
	Device& operator=(Device&&) = delete;
};

}  // namespace ignis
