#pragma once

#include <vk_mem_alloc.h>
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

// PONDER probaly device should inherit a pointer class
class Device {
public:
	struct CreateInfo {
		std::string appName{"Ignis App"};
		std::string shadersFolder{"shaders"};
		std::vector<std::string> extensions{};
	};

	Device(CreateInfo);
	~Device();

	bool getQueue(uint32_t index, VkQueue*) const;

	struct SubmitInfo {
		std::vector<const Semaphore*> waitSemaphores;
		std::vector<const Semaphore*> signalSemaphores;
		const Command* command;
		Fence* fence;
	};

	void submitCommands(std::vector<SubmitInfo>) const;

	VkCommandPool getCommandPool();

	VkDevice getDevice() const { return m_device; }

	VmaAllocator getAllocator() const { return m_allocator; }

	// will return the full shaderPath only if shaderPath is not an absolute path
	std::string getFullShaderPath(std::string shaderPath) const;

private:
	VkInstance m_instance{nullptr};
	VkDebugUtilsMessengerEXT m_debugMessenger{nullptr};
	VkPhysicalDevice m_phyiscalDevice{nullptr};
	VkDevice m_device{nullptr};
	VmaAllocator m_allocator{nullptr};

	std::vector<VkQueue> m_queues;
	std::vector<VkCommandPool> m_pools;

	std::string m_shadersFolder;

public:
	Device(const Device&) = default;
	Device(Device&&) = delete;
	Device& operator=(const Device&) = default;
	Device& operator=(Device&&) = delete;
};

}  // namespace ignis
