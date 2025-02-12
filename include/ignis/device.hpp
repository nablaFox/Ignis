#pragma once

#include <vulkan/vulkan_core.h>
#include <string>
#include <vector>

namespace ignis {

class Semaphore;
class Fence;
class Command;

// Note 1: the library supports just 1 instance, physical and logical device
// Note 2: we handle only graphics queues
// Note 3: the library works only in vulkan 1.3 with dynamic rendering and other
// required features
// Note 4: we don't handle custom features/extensions

class Device {
public:
	struct CreateDeviceInfo {
		std::vector<std::string> extensions;
	};

	Device(CreateDeviceInfo);
	~Device();

	bool getQueue(uint32_t index, VkQueue*) const;

	void submitCommands(const std::vector<Command>&,
						const std::vector<Semaphore>& waitSemaphores,
						const std::vector<Semaphore>& signalSemaphores,
						const Fence&);

	VkCommandPool getCommandPool();

	VkDevice getDevice() const { return m_device; }

private:
	VkInstance m_instance{nullptr};
	VkDebugUtilsMessengerEXT m_debugMessenger{nullptr};
	VkPhysicalDevice m_phyiscalDevice{nullptr};
	VkDevice m_device{nullptr};
	std::vector<VkQueue> m_queues;
	std::vector<VkCommandPool> m_pools;

public:
	Device(const Device&) = default;
	Device(Device&&) = delete;
	Device& operator=(const Device&) = default;
	Device& operator=(Device&&) = delete;
};

}  // namespace ignis
