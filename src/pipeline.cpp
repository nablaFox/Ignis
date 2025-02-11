#include "ignis/pipeline.hpp"
#include "ignis/shader.hpp"
#include "ignis/descriptor_set_layout.hpp"
#include "ignis/pipeline_layout.hpp"
#include "ignis/device.hpp"
#include "ignis/exceptions.hpp"
#include "ignis/vertex_input_layout.hpp"

using namespace ignis;

Pipeline::Pipeline(Device& device,
				   std::vector<Shader> shaders,
				   ColorFormat colorFormat,
				   DepthFormat depthFormat,
				   const VertexInputLayout& vertexInputLayout)
	: m_device(device) {
	m_pipelineLayout = std::make_unique<PipelineLayout>(m_device, shaders);

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages(shaders.size());

	for (const auto& shader : shaders)
		shaderStages.push_back({
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = shader.getStage(),
			.module = shader.getModule(),
			.pName = "main",
		});

	auto vertexBindings = vertexInputLayout.getBindings();
	auto vertexAttributes = vertexInputLayout.getAttributes();

	VkPipelineVertexInputStateCreateInfo vertexInput{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount =
			static_cast<uint32_t>(vertexBindings.size()),
		.pVertexBindingDescriptions = vertexBindings.data(),
		.vertexAttributeDescriptionCount =
			static_cast<uint32_t>(vertexAttributes.size()),
		.pVertexAttributeDescriptions = vertexAttributes.data(),
	};

	VkPipelineInputAssemblyStateCreateInfo inputAssembly{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.primitiveRestartEnable = VK_FALSE,
	};

	VkPipelineViewportStateCreateInfo viewportState{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.viewportCount = 1,
		.scissorCount = 1,
	};

	VkPipelineRasterizationStateCreateInfo rasterizer{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.depthClampEnable = VK_FALSE,
		.rasterizerDiscardEnable = VK_FALSE,
		.polygonMode = VK_POLYGON_MODE_FILL,
		.cullMode = VK_CULL_MODE_BACK_BIT,
		.frontFace = VK_FRONT_FACE_CLOCKWISE,
		.lineWidth = 1.0f,
		.depthBiasEnable = VK_FALSE,
	};

	VkPipelineMultisampleStateCreateInfo multisampling{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
		.sampleShadingEnable = VK_FALSE,
	};

	VkPipelineDepthStencilStateCreateInfo depthStencil{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
		.depthTestEnable = VK_TRUE,
		.depthWriteEnable = VK_TRUE,
		.depthCompareOp = VK_COMPARE_OP_LESS,
		.depthBoundsTestEnable = VK_FALSE,
		.stencilTestEnable = VK_FALSE,
	};

	VkPipelineColorBlendAttachmentState colorBlendAttachment{
		.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
						  VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
		.blendEnable = VK_FALSE,
	};

	VkPipelineColorBlendStateCreateInfo colorBlending{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.logicOpEnable = VK_FALSE,
		.attachmentCount = 1,
		.pAttachments = &colorBlendAttachment,
	};

	std::vector<VkDynamicState> dynamicStates = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR,
	};

	VkPipelineDynamicStateCreateInfo dynamicState{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
		.pDynamicStates = dynamicStates.data(),
	};

	VkFormat vkColorFormat = static_cast<VkFormat>(colorFormat);
	VkFormat vkDepthFormat = static_cast<VkFormat>(depthFormat);

	VkPipelineRenderingCreateInfo pipelineRenderingInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
		.colorAttachmentCount = 1,
		.pColorAttachmentFormats = &vkColorFormat,
		.depthAttachmentFormat = vkDepthFormat,
		.stencilAttachmentFormat = VK_FORMAT_UNDEFINED,
	};

	VkGraphicsPipelineCreateInfo pipelineInfo{
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
		.layout = m_pipelineLayout->getHandle(),
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
