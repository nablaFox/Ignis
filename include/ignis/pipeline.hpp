#pragma once

#include <vulkan/vulkan_core.h>
#include <string>
#include <vector>

namespace ignis {

class Device;
class Shader;
enum class ColorFormat;
enum class DepthFormat;

// Note 1: for now we handle only graphics pipelines
// Note 2: we can't render to multiple images, just to a single one
// Note 3: dynamic rendering only
// Note 4: vertexInput, inputAssembly & dynamicStates are not configurable

class Pipeline {
public:
	struct CreateInfo {
		const Device* device;
		std::vector<std::string> shaders;
		ColorFormat colorFormat;
		DepthFormat depthFormat;
		VkCullModeFlagBits cullMode{VK_CULL_MODE_BACK_BIT};
		VkFrontFace frontFace{VK_FRONT_FACE_CLOCKWISE};
		VkPolygonMode polygonMode{VK_POLYGON_MODE_FILL};
		VkCompareOp depthCompareOp{VK_COMPARE_OP_LESS};
		float lineWidth{1.0f};
		VkSampleCountFlagBits sampleCount{VK_SAMPLE_COUNT_1_BIT};
		bool sampleShadingEnable{false};
		float minSampleShading{1.0f};
		bool enableDepthTest{false};
		bool enableDepthWrite{false};
		bool blendEnable{false};
		VkBlendFactor srcColorBlendFactor{VK_BLEND_FACTOR_ONE};
		VkBlendFactor dstColorBlendFactor{VK_BLEND_FACTOR_ZERO};
		VkBlendOp colorBlendOp{VK_BLEND_OP_ADD};
		VkBlendFactor srcAlphaBlendFactor{VK_BLEND_FACTOR_ONE};
		VkBlendFactor dstAlphaBlendFactor{VK_BLEND_FACTOR_ZERO};
		VkBlendOp alphaBlendOp{VK_BLEND_OP_ADD};
	};

	Pipeline(CreateInfo);

	~Pipeline();

	VkPipeline getHandle() const { return m_pipeline; }

	VkPipelineLayout getLayoutHandle() const { return m_pipelineLayout; }

private:
	const Device& m_device;
	VkPipelineLayout m_pipelineLayout{VK_NULL_HANDLE};
	VkPipeline m_pipeline{VK_NULL_HANDLE};

public:
	Pipeline(const Pipeline&) = delete;
	Pipeline(Pipeline&&) = delete;
	Pipeline& operator=(const Pipeline&) = delete;
	Pipeline& operator=(Pipeline&&) = delete;
};

}  // namespace ignis
