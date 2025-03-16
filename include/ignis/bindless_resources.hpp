#include <vulkan/vulkan_core.h>
#include <array>

namespace ignis {

static inline constexpr uint32_t MAX_PUSH_CONSTANT_WORD_SIZE = 32;

static inline constexpr uint32_t PIPELINE_LAYOUT_COUNT =
	MAX_PUSH_CONSTANT_WORD_SIZE + 1;  // we account also for no push constants

struct BindlessResourcesCreateInfo {
	VkDevice device{nullptr};
	uint32_t maxStorageBuffers{0};
	uint32_t maxUniformBuffers{0};
	uint32_t maxImageSamplers{0};
	uint32_t storageBuffersBinding{0};
	uint32_t uniformBuffersBinding{1};
	uint32_t imageSamplersBinding{2};
};

struct BindlessResources {
	BindlessResources(const BindlessResourcesCreateInfo&);
	~BindlessResources();

	VkPipelineLayout getPipelinelayout(uint32_t pushConstantSize) const {
		return m_pipelineLayouts.at(pushConstantSize);
	}

	VkDescriptorSet getDescriptorSet() const { return m_descriptorSet; }

	void registerBuffer(VkBuffer buffer,
						VkBufferUsageFlags usage,
						VkDeviceSize size,
						uint32_t binding) const;

	void registerSampledImage(VkImageView imageView,
							  VkSampler sampler,
							  uint32_t binding);

private:
	BindlessResourcesCreateInfo m_creationInfo;

	VkDescriptorSetLayout m_descriptorSetLayout{nullptr};
	VkDescriptorPool m_descriptorPool{nullptr};
	VkDescriptorSet m_descriptorSet{nullptr};
	std::array<VkPipelineLayout, PIPELINE_LAYOUT_COUNT> m_pipelineLayouts{};

public:
	BindlessResources(const BindlessResources&) = delete;
	BindlessResources(BindlessResources&&) = delete;
	BindlessResources& operator=(const BindlessResources&) = delete;
	BindlessResources& operator=(BindlessResources&&) = delete;
};

}  // namespace ignis
