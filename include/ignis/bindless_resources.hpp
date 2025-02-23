#include <vulkan/vulkan_core.h>
#include <array>

namespace ignis {

#define IGNIS_STORAGE_BUFFER_BINDING 0
#define IGNIS_UNIFORM_BUFFER_BINDING 1
#define IGNIS_IMAGE_SAMPLER_BINDING 2

static inline constexpr uint32_t MAX_PUSH_CONSTANT_WORD_SIZE = 32;

static inline constexpr uint32_t PIPELINE_LAYOUT_COUNT =
	MAX_PUSH_CONSTANT_WORD_SIZE + 1;  // we account also for no push constants

struct BindlessResources {
	void initialize(VkDevice);
	void cleanup(VkDevice);

	VkPipelineLayout getPipelinelayout(uint32_t pushConstantSize) const {
		return m_pipelineLayouts.at(pushConstantSize);
	}

private:
	VkDescriptorSetLayout m_descriptorSetLayout{nullptr};
	VkDescriptorPool m_descriptorPool{nullptr};
	VkDescriptorSet m_descriptorSet{nullptr};

	std::array<VkPipelineLayout, PIPELINE_LAYOUT_COUNT> m_pipelineLayouts{};
};

}  // namespace ignis
