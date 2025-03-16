#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include "types.hpp"

namespace ignis {

class GpuResources;
class Semaphore;
class Fence;
class Command;
class Buffer;
struct BufferCreateInfo;
class Image;
struct ImageCreateInfo;
struct DrawImageCreateInfo;
struct DepthImageCreateInfo;
class Sampler;
class Features;

struct SubmitCmdInfo {
	const Command& command;
	std::vector<const Semaphore*> waitSemaphores;
	std::vector<const Semaphore*> signalSemaphores;
};

// Note 1: the library supports just 1 instance, physical and logical device
// Note 2: we handle only graphics queues
// Note 3: the library works only in vulkan 1.3 with dynamic rendering and other
// required features
// Note 4: we don't handle optional features
// Note 5: command pools are relative to a single thread
// Note 6: we allocate a command pool for each queue
// Note 7: only combined image samplers are supported

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

	bool isFeatureEnabled(const char* featureName) const;

	VkSampleCountFlagBits getMaxSampleCount() const;

	VmaAllocator getAllocator() const { return m_allocator; }

	std::string getShadersFolder() const { return m_shadersFolder; };

public:
	// will always be registered (the binding will depend if ubo or ssbo)
	// it will be asserted that ubo or ssbo
	BufferId createBuffer(const BufferCreateInfo&,
						  BufferId = IGNIS_INVALID_BUFFER_ID);

	// ubo with default specs
	BufferId createUBO(VkDeviceSize size,
					   const void* data = nullptr,
					   BufferId = IGNIS_INVALID_BUFFER_ID);

	// ssbo with default specs
	BufferId createSSBO(VkDeviceSize size,
						const void* data = nullptr,
						BufferId = IGNIS_INVALID_BUFFER_ID);

	Buffer createStagingBuffer(VkDeviceSize, const void* data = nullptr);

	Buffer& getBuffer(BufferId) const;

	void destroyBuffer(BufferId);

public:
	// it will be registered only if storage or sampled
	ImageId createImage(const ImageCreateInfo&, ImageId = IGNIS_INVALID_IMAGE_ID);

	ImageId createDrawImage(const DrawImageCreateInfo&);

	ImageId createDepthImage(const DepthImageCreateInfo&);

	// will assert that the image is registered
	Image& getImage(ImageId) const;

	void destroyImage(ImageId);

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

	std::unique_ptr<GpuResources> m_bindlessResources;

	std::string m_shadersFolder;

	std::unique_ptr<Features> m_features;

private:
	std::unordered_map<BufferId, std::unique_ptr<Buffer>> m_buffers;
	std::unordered_map<ImageId, std::unique_ptr<Image>> m_images;

public:
	Device(const Device&) = delete;
	Device(Device&&) = delete;
	Device& operator=(const Device&) = delete;
	Device& operator=(Device&&) = delete;
};

}  // namespace ignis
