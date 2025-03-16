#include <cassert>
#include <memory>
#include "pipeline.hpp"
#include "shader.hpp"
#include "device.hpp"
#include "exceptions.hpp"

using namespace ignis;

Pipeline::Pipeline(const PipelineCreateInfo& info) : m_device(*info.device) {
	std::vector<std::unique_ptr<Shader>> shaders;

	assert(!info.shaders.empty() && "No shaders provided");

	for (const auto& shaderPath : info.shaders) {
		shaders.emplace_back(std::make_unique<Shader>(m_device, shaderPath));
	}

	ShaderResources resources{};

	for (const auto& shader : shaders) {
		Shader::getMergedResources(shader->getResources(), &resources);
	}

	m_pipelineLayout = m_device.getPipelineLayout(resources.pushConstants.size);

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
	shaderStages.reserve(info.shaders.size());

	for (const auto& shader : shaders) {
		shaderStages.push_back({
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = shader->getStage(),
			.module = shader->getModule(),
			.pName = "main",
		});
	}

	VkPipelineVertexInputStateCreateInfo const vertexInput{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = 0,
		.pVertexBindingDescriptions = nullptr,
		.vertexAttributeDescriptionCount = 0,
		.pVertexAttributeDescriptions = nullptr,
	};

	VkPipelineInputAssemblyStateCreateInfo const inputAssembly{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.primitiveRestartEnable = VK_FALSE,
	};

	VkPipelineViewportStateCreateInfo const viewportState{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.viewportCount = 1,
		.scissorCount = 1,
	};

	VkPipelineRasterizationStateCreateInfo const rasterizer{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.depthClampEnable = VK_FALSE,
		.rasterizerDiscardEnable = VK_FALSE,
		.polygonMode = info.polygonMode,
		.cullMode = info.cullMode,
		.frontFace = info.frontFace,
		.depthBiasEnable = VK_FALSE,
		.lineWidth = info.lineWidth,
	};

	VkPipelineMultisampleStateCreateInfo const multisampling{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.rasterizationSamples = info.sampleCount,
		.sampleShadingEnable = info.sampleShadingEnable,
		.minSampleShading = info.minSampleShading,
	};

	VkPipelineDepthStencilStateCreateInfo const depthStencil{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
		.depthTestEnable = VK_TRUE,
		.depthWriteEnable = info.enableDepthWrite,
		.depthCompareOp = info.depthCompareOp,
		.depthBoundsTestEnable = VK_FALSE,
		.stencilTestEnable = VK_FALSE,
	};

	VkPipelineColorBlendAttachmentState const colorBlendAttachment{
		.blendEnable = info.blendEnable,
		.srcColorBlendFactor = info.srcColorBlendFactor,
		.dstColorBlendFactor = info.dstColorBlendFactor,
		.colorBlendOp = info.colorBlendOp,
		.srcAlphaBlendFactor = info.srcAlphaBlendFactor,
		.dstAlphaBlendFactor = info.dstAlphaBlendFactor,
		.alphaBlendOp = info.alphaBlendOp,
		.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
						  VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
	};

	VkPipelineColorBlendStateCreateInfo const colorBlending{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.logicOpEnable = VK_FALSE,
		.attachmentCount = 1,
		.pAttachments = &colorBlendAttachment,
	};

	std::vector<VkDynamicState> const dynamicStates{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR,
	};

	VkPipelineDynamicStateCreateInfo const dynamicState{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
		.pDynamicStates = dynamicStates.data(),
	};

	VkFormat vkColorFormat = static_cast<VkFormat>(info.colorFormat);
	VkFormat vkDepthFormat = static_cast<VkFormat>(info.depthFormat);

	VkPipelineRenderingCreateInfo pipelineRenderingInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
		.colorAttachmentCount = 1,
		.pColorAttachmentFormats = &vkColorFormat,
		.stencilAttachmentFormat = VK_FORMAT_UNDEFINED,
	};

	if (info.enableDepthTest) {
		pipelineRenderingInfo.depthAttachmentFormat = vkDepthFormat;
	}

	VkGraphicsPipelineCreateInfo const pipelineInfo{
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.pNext = &pipelineRenderingInfo,
		.stageCount = static_cast<uint32_t>(shaderStages.size()),
		.pStages = shaderStages.data(),
		.pVertexInputState = &vertexInput,
		.pInputAssemblyState = &inputAssembly,
		.pViewportState = &viewportState,
		.pRasterizationState = &rasterizer,
		.pMultisampleState = &multisampling,
		.pDepthStencilState = &depthStencil,
		.pColorBlendState = &colorBlending,
		.pDynamicState = &dynamicState,
		.layout = m_pipelineLayout,
		.basePipelineHandle = VK_NULL_HANDLE,
	};

	THROW_VULKAN_ERROR(
		vkCreateGraphicsPipelines(m_device.getDevice(), VK_NULL_HANDLE, 1,
								  &pipelineInfo, nullptr, &m_pipeline),
		"Failed to create pipeline");
}

Pipeline::~Pipeline() {
	vkDestroyPipeline(m_device.getDevice(), m_pipeline, nullptr);
}
