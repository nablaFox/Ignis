#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <array>
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
class Shader;

struct SubmitCmdInfo {
	const Command& command;
	std::vector<const Semaphore*> waitSemaphores;
	std::vector<const Semaphore*> signalSemaphores;
};

inline constexpr std::array IGNIS_REQ_FEATURES = {
	"BufferDeviceAddress",
	"DynamicRendering",
	"Synchronization2",
	"DescriptorBindingUniformBufferUpdateAfterBind",
	"DescriptorBindingSampledImageUpdateAfterBind",
	"DescriptorBindingStorageBufferUpdateAfterBind",
	"DescriptorBindingPartiallyBound",
	"RuntimeDescriptorArray",
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
		std::vector<const char*> extensions{};
		std::vector<const char*> instanceExtensions{};
		std::vector<const char*> requiredFeatures{};
		std::vector<const char*> optionalFeatures{};
	};

	Device(const CreateInfo&);

	~Device();

	auto getPhysicalDevice() const { return m_phyiscalDevice; }

	auto getInstance() const { return m_instance; }

	auto getDevice() const { return m_device; }

	auto getAllocator() const { return m_allocator; }

	auto getPhysicalDeviceProperties() const { return m_physicalDeviceProperties; }

	auto getQueueCount() const { return m_graphicsQueuesCount; }

public:
	VkQueue getQueue(uint32_t index) const;

	VkCommandPool getCommandPool(VkQueue) const;

	void submitCommands(std::vector<SubmitCmdInfo>, const Fence* fence) const;

	VkSampleCountFlagBits getMaxSampleCount() const;

	bool isFeatureEnabled(const char* featureName) const;

	void waitIdle() const;

	Buffer createStagingBuffer(VkDeviceSize, const void* data = nullptr) const;

	Buffer createIndexBuffer32(uint32_t elementCount,
							   const uint32_t* data = nullptr) const;

	Image createDrawAttachmentImage(const DrawImageCreateInfo&) const;

	Image createDepthAttachmentImage(const DepthImageCreateInfo&) const;

	Shader createShader(const std::string& shaderPath,
						VkShaderStageFlagBits,
						size_t pushConstansSize = 0) const;

public:
	BufferId createUBO(VkDeviceSize, const void* data = nullptr) const;

	BufferId createSSBO(VkDeviceSize, const void* data = nullptr) const;

	ImageId createStorageImage(const ImageCreateInfo&) const;

	ImageId createSampledImage(const ImageCreateInfo&) const;

	Buffer& getBuffer(BufferId) const;

	Image& getImage(ImageId) const;

	void destroyBuffer(BufferId);

	void destroyImage(ImageId);

	void updateBuffer(BufferId,
					  const void* data,
					  VkDeviceSize offset = 0,
					  VkDeviceSize size = 0) const;

	VkPipelineLayout getPipelineLayout(VkDeviceSize pushConstantSize) const;

	VkDescriptorSet getDescriptorSet() const;

private:
	VkInstance m_instance{nullptr};
	VkDebugUtilsMessengerEXT m_debugMessenger{nullptr};
	VkPhysicalDevice m_phyiscalDevice{nullptr};
	VkPhysicalDeviceProperties m_physicalDeviceProperties{};
	VkDevice m_device{nullptr};
	VmaAllocator m_allocator{nullptr};
	std::unique_ptr<Features> m_features;
	std::unique_ptr<GpuResources> m_gpuResources;

	uint32_t m_graphicsFamilyIndex{0};
	uint32_t m_graphicsQueuesCount{0};
	std::vector<VkQueue> m_queues;
	std::unordered_map<VkQueue, VkCommandPool> m_commandPools;

public:
	Device(const Device&) = delete;
	Device(Device&&) = delete;
	Device& operator=(const Device&) = delete;
	Device& operator=(Device&&) = delete;
};

}  // namespace ignis
