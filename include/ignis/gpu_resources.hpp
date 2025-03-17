#include <vulkan/vulkan_core.h>
#include <array>
#include <memory>
#include <unordered_map>
#include "types.hpp"

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

class Buffer;
class Image;

struct GpuResources {
	GpuResources(const BindlessResourcesCreateInfo&);

	~GpuResources();

	auto getPipelinelayout(uint32_t pushConstantSize) const {
		return m_pipelineLayouts.at(pushConstantSize);
	}

	auto getDescriptorSet() const { return m_descriptorSet; }

	BufferId registerBuffer(Buffer buffer, BufferId = IGNIS_INVALID_BUFFER_ID);

	ImageId registerImage(Image image, ImageId = IGNIS_INVALID_IMAGE_ID);

	Buffer& getBuffer(BufferId) const;

	Image& getImage(ImageId) const;

	void destroyBuffer(BufferId&);

	void destroyImage(ImageId&);

private:
	VkDevice m_device;
	BindlessResourcesCreateInfo m_creationInfo;

	VkDescriptorSetLayout m_descriptorSetLayout{nullptr};
	VkDescriptorPool m_descriptorPool{nullptr};
	VkDescriptorSet m_descriptorSet{nullptr};
	std::array<VkPipelineLayout, PIPELINE_LAYOUT_COUNT> m_pipelineLayouts{};

	std::unordered_map<BufferId, std::unique_ptr<Buffer>> m_buffers;
	std::unordered_map<ImageId, std::unique_ptr<Image>> m_images;
	BufferId nextBufferId{0};
	ImageId nextImageId{0};

public:
	GpuResources(const GpuResources&) = delete;
	GpuResources(GpuResources&&) = delete;
	GpuResources& operator=(const GpuResources&) = delete;
	GpuResources& operator=(GpuResources&&) = delete;
};

}  // namespace ignis
