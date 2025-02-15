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
// Note 5: command pools are relative to a single thread
// Note 6: we allocate a command pool for each queue

// PONDER probaly device should inherit a pointer class
class Device {
public:
	struct CreateInfo {
		std::string appName{"Ignis App"};
		std::string shadersFolder{"shaders"};
		std::vector<const char*> extensions{};
		std::vector<const char*> instanceExtensions{};
	};

	Device(CreateInfo);
	~Device();

	// each command should be relative to the same queue
	struct SubmitInfo {
		const Command* command;
		std::vector<const Semaphore*> waitSemaphores;
		std::vector<const Semaphore*> signalSemaphore;
	};

	void submitCommands(std::vector<SubmitInfo>, const Fence& fence) const;

	VkPhysicalDevice getPhysicalDevice() const { return m_phyiscalDevice; }

	bool getQueue(uint32_t index, VkQueue*) const;

	bool getCommandPool(uint32_t index, VkCommandPool*) const;

	VkDevice getDevice() const { return m_device; }

	VkDeviceSize getUboAlignment() const {
		return m_physicalDeviceProperties.limits.minUniformBufferOffsetAlignment;
	}

	VkDeviceSize getSsboAlignment() const {
		return m_physicalDeviceProperties.limits.minStorageBufferOffsetAlignment;
	}

	VmaAllocator getAllocator() const { return m_allocator; }

	// will return the full shaderPath only if shaderPath is not an absolute path
	std::string getFullShaderPath(std::string shaderPath) const;

	PFN_vkCmdPushDescriptorSetKHR getPushDescriptorFunc() const {
		return m_vkCmdPushDescriptorSetKHR;
	}

private:
	VkInstance m_instance{nullptr};
	VkDebugUtilsMessengerEXT m_debugMessenger{nullptr};
	VkPhysicalDevice m_phyiscalDevice{nullptr};
	VkPhysicalDeviceProperties m_physicalDeviceProperties{};
	VkDevice m_device{nullptr};
	VmaAllocator m_allocator{nullptr};
	PFN_vkCmdPushDescriptorSetKHR m_vkCmdPushDescriptorSetKHR{nullptr};

	uint32_t m_graphicsFamilyIndex{0};
	uint32_t m_graphicsQueuesCount{0};
	std::vector<VkQueue> m_queues;
	std::vector<VkCommandPool> m_commandPools;

	std::string m_shadersFolder;

public:
	Device(const Device&) = default;
	Device(Device&&) = delete;
	Device& operator=(const Device&) = default;
	Device& operator=(Device&&) = delete;
};

}  // namespace ignis
