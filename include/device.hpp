#pragma once

#include <vulkan/vulkan_core.h>
#include <vector>

namespace ignis {

class Semaphore;
class Fence;
class Command;

struct Queue {
	VkQueue queue;
	VkQueueFlags type;
	uint32_t family;
	uint32_t index;
};

// Note 1: the library will support just 1 instance, physical and logical device
// Note 2: for now we handle only graphics queues

class Device {
public:
	Device();
	~Device();

	bool getQueue(uint32_t index, Queue*) const;

	void submitCommands(const std::vector<Command>&,
						const std::vector<Semaphore>& waitSemaphores,
						const std::vector<Semaphore>& signalSemaphores,
						const Fence&);

	VkCommandPool getCommandPool(Queue);

private:
	std::vector<VkCommandPool> m_pools;
	std::vector<std::vector<Queue>> m_queues;

public:
	Device(const Device&) = default;
	Device(Device&&) = delete;
	Device& operator=(const Device&) = default;
	Device& operator=(Device&&) = delete;
};

}  // namespace ignis
