#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>
#include <memory>
#include <string>
#include <vector>
#include "bindless_resources.hpp"

namespace ignis {

class Semaphore;
class Fence;
class Command;
class Buffer;
class Image;
class Sampler;
class Features;

// each command should be relative to the same queue
struct SubmitCmdInfo {
	const Command* command;
	std::vector<const Semaphore*> waitSemaphores;
	std::vector<const Semaphore*> signalSemaphores;
};

constexpr std::array IGNIS_REQ_FEATURES = {
	"BufferDeviceAddress",
	"DynamicRendering",
	"Synchronization2",
	"DescriptorIndexing",
};

// Note 1: the library supports just 1 instance, physical and logical device
// Note 2: we handle only graphics queues
// Note 3: the library works only in vulkan 1.3 with dynamic rendering and other
// required features
// Note 4: we don't handle optional features
// Note 5: command pools are relative to a single thread
// Note 6: we allocate a command pool for each queue
// Note 7: only combined image samplers are supported

// PONDER probaly device should inherit a pointer class
class Device {
public:
	struct CreateInfo {
		std::string appName{"Ignis App"};
		std::string shadersFolder{"shaders"};
		std::vector<const char*> extensions{};
		std::vector<const char*> instanceExtensions{};
		std::vector<const char*> requiredFeatures{};
		std::vector<const char*> optionalFeatures{};
	};

	Device(CreateInfo);
	~Device();

	void submitCommands(std::vector<SubmitCmdInfo>, const Fence& fence) const;

	VkPhysicalDevice getPhysicalDevice() const { return m_phyiscalDevice; }

	VkInstance getInstance() const { return m_instance; }

	VkDevice getDevice() const { return m_device; }

	VkQueue getQueue(uint32_t index) const;

	VkCommandPool getCommandPool(uint32_t index) const;

	VkDeviceSize getUboAlignment() const {
		return m_physicalDeviceProperties.limits.minUniformBufferOffsetAlignment;
	}

	VkDeviceSize getSsboAlignment() const {
		return m_physicalDeviceProperties.limits.minStorageBufferOffsetAlignment;
	}

	VkPipelineLayout getPipelineLayout(uint32_t pushConstantSize) const {
		return m_bindlessResources.getPipelinelayout(1 + (pushConstantSize / 4));
	}

	VkDescriptorSet getDescriptorSet() const {
		return m_bindlessResources.getDescriptorSet();
	}

	void registerUBO(const Buffer& buffer, uint32_t index);

	void registerSSBO(const Buffer& buffer, uint32_t index);

	void registerSampledImage(const Image& image,
							  const Sampler& sampler,
							  uint32_t index);

	bool isFeatureEnabled(const char* feature) const;

	VkSampleCountFlagBits getMaxSampleCount() const;

	VmaAllocator getAllocator() const { return m_allocator; }

	std::string getFullShaderPath(std::string shaderPath) const;

private:
	VkInstance m_instance{nullptr};
	VkDebugUtilsMessengerEXT m_debugMessenger{nullptr};
	VkPhysicalDevice m_phyiscalDevice{nullptr};
	VkPhysicalDeviceProperties m_physicalDeviceProperties{};
	VkDevice m_device{nullptr};
	VmaAllocator m_allocator{nullptr};

	uint32_t m_graphicsFamilyIndex{0};
	uint32_t m_graphicsQueuesCount{0};
	std::vector<VkQueue> m_queues;
	std::vector<VkCommandPool> m_commandPools;

	BindlessResources m_bindlessResources;

	std::string m_shadersFolder;

	std::unique_ptr<Features> m_features;

public:
	Device(const Device&) = delete;
	Device(Device&&) = delete;
	Device& operator=(const Device&) = delete;
	Device& operator=(Device&&) = delete;
};

}  // namespace ignis
