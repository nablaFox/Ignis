#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <array>
#include <stdint.h>

namespace ignis {

class BindlessResources;
class Semaphore;
class Fence;
class Command;
class Buffer;
struct BufferCreateInfo;
class Image;
class Sampler;
class Features;

#define IGNIS_STORAGE_BUFFER_BINDING 0
#define IGNIS_UNIFORM_BUFFER_BINDING 1
#define IGNIS_IMAGE_SAMPLER_BINDING 2

#define IGNIS_INVALID_BUFFER_ID UINT32_MAX

struct SubmitCmdInfo {
	const Command& command;
	std::vector<const Semaphore*> waitSemaphores;
	std::vector<const Semaphore*> signalSemaphores;
};

constexpr std::array IGNIS_REQ_FEATURES = {
	"BufferDeviceAddress",
	"DynamicRendering",
	"Synchronization2",
	"DescriptorBindingUniformBufferUpdateAfterBind",
	"DescriptorBindingSampledImageUpdateAfterBind",
	"DescriptorBindingStorageBufferUpdateAfterBind",
	"DescriptorBindingPartiallyBound",
	"RuntimeDescriptorArray",
};

typedef uint32_t BufferId;

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

	Device(const CreateInfo&);

	~Device();

	void submitCommands(std::vector<SubmitCmdInfo>, const Fence* fence) const;

	VkPhysicalDevice getPhysicalDevice() const { return m_phyiscalDevice; }

	VkInstance getInstance() const { return m_instance; }

	VkDevice getDevice() const { return m_device; }

	VkQueue getQueue(uint32_t index) const;

	VkCommandPool getCommandPool(VkQueue) const;

	VkDeviceSize getUboAlignment() const {
		return m_physicalDeviceProperties.limits.minUniformBufferOffsetAlignment;
	}

	VkDeviceSize getSsboAlignment() const {
		return m_physicalDeviceProperties.limits.minStorageBufferOffsetAlignment;
	}

	VkPipelineLayout getPipelineLayout(uint32_t pushConstantSize) const;

	VkDescriptorSet getDescriptorSet() const;

	Buffer& getBuffer(BufferId) const;

	void destroyBuffer(BufferId);

	// will not be registered
	BufferId createBuffer(const BufferCreateInfo&,
						  BufferId = IGNIS_INVALID_BUFFER_ID);

	// will not be registered
	BufferId createIndexBuffer32(uint32_t elementCount,
								 BufferId = IGNIS_INVALID_BUFFER_ID);

	BufferId createUBO(VkDeviceSize,
					   const void* data = nullptr,
					   BufferId = IGNIS_INVALID_BUFFER_ID);

	BufferId createSSBO(VkDeviceSize, BufferId = IGNIS_INVALID_BUFFER_ID);

	// will not be saved or registered
	Buffer createStagingBuffer(VkDeviceSize, const void* data = nullptr);

	void registerSampledImage(const Image&, const Sampler&, uint32_t index);

	bool isFeatureEnabled(const char* featureName) const;

	VkSampleCountFlagBits getMaxSampleCount() const;

	VmaAllocator getAllocator() const { return m_allocator; }

	std::string getShadersFolder() const { return m_shadersFolder; };

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
	std::unordered_map<VkQueue, VkCommandPool> m_commandPools;

	std::unique_ptr<BindlessResources> m_bindlessResources;

	std::string m_shadersFolder;

	std::unique_ptr<Features> m_features;

private:
	std::unordered_map<BufferId, std::unique_ptr<Buffer>> m_buffers;

public:
	Device(const Device&) = delete;
	Device(Device&&) = delete;
	Device& operator=(const Device&) = delete;
	Device& operator=(Device&&) = delete;
};

}  // namespace ignis
