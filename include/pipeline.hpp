#pragma once

#include <vulkan/vulkan_core.h>
#include <memory>
#include <vector>

namespace ignis {

class Device;
class Shader;
class DescriptorSetLayout;
class PipelineLayout;
enum class ColorFormat;
enum class DepthFormat;

// Note 1: for now we handle only graphics pipelines
// Note 2: we can't render to multiple images, just to a single one
// Note 3: only dynamic rendering is available
// Note 4: no state is configurable apart from the attachment formats
// Note 5: if two pipelines have the same layout the implementation will allocate
// 2 different PipelineLayout objects

class Pipeline {
protected:
	Pipeline(Device&, std::vector<Shader>, ColorFormat, DepthFormat);
	~Pipeline();

private:
	Device& m_device;
	std::unique_ptr<PipelineLayout> m_pipelineLayout;
	VkPipeline m_pipeline{VK_NULL_HANDLE};

public:
	Pipeline(const Pipeline&) = delete;
	Pipeline(Pipeline&&) = delete;
	Pipeline& operator=(const Pipeline&) = delete;
	Pipeline& operator=(Pipeline&&) = delete;
};

}  // namespace ignis
