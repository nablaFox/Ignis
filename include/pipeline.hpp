#pragma once

#include <vulkan/vulkan_core.h>
#include <memory>
#include <unordered_map>
#include <vector>

namespace ignis {

class Device;
class Shader;
class DescriptorSetLayout;
class PipelineLayout;

// Note 1: for now we handle only graphics pipelines

class Pipeline {
protected:
	Pipeline(Device&, std::vector<Shader>);
	~Pipeline();

private:
	Device& m_device;
	std::unique_ptr<PipelineLayout> m_pipelineLayout;
	std::unordered_map<uint32_t, DescriptorSetLayout> m_descriptorSetLayouts;
	VkPipeline m_pipeline;

public:
	Pipeline(const Pipeline&) = delete;
	Pipeline(Pipeline&&) = delete;
	Pipeline& operator=(const Pipeline&) = delete;
	Pipeline& operator=(Pipeline&&) = delete;
};

}  // namespace ignis
