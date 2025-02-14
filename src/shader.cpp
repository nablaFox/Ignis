#include "shader.hpp"
#include "device.hpp"
#include "descriptor_set_layout.hpp"
#include "exceptions.hpp"

#include <fstream>
#include <spirv_glsl.hpp>
#include <spirv_cross.hpp>

using namespace ignis;

Shader::Shader(const Device& device, std::string shaderPath) : m_device(device) {
	std::string fullShaderPath = device.getFullShaderPath(shaderPath);

	std::ifstream file(fullShaderPath, std::ios::ate | std::ios::binary);

	THROW_ERROR(!file.is_open(), "Failed to open shader file " + fullShaderPath);

	size_t fileSize = static_cast<size_t>(file.tellg());
	std::vector<uint32_t> code(fileSize / sizeof(uint32_t));
	file.seekg(0);
	file.read(reinterpret_cast<char*>(code.data()), fileSize);
	file.close();

	VkShaderModuleCreateInfo moduleCreateInfo{
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = code.size() * sizeof(uint32_t),
		.pCode = code.data(),
	};

	THROW_VULKAN_ERROR(vkCreateShaderModule(device.getDevice(), &moduleCreateInfo,
											nullptr, &m_module),
					   "Failed to create shader module");

	spirv_cross::Compiler compiler(code);
	auto executionModel = compiler.get_execution_model();

	switch (executionModel) {
		case spv::ExecutionModelVertex:
			m_stage = VK_SHADER_STAGE_VERTEX_BIT;
			break;
		case spv::ExecutionModelFragment:
			m_stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			break;
		case spv::ExecutionModelGLCompute:
			m_stage = VK_SHADER_STAGE_COMPUTE_BIT;
			break;
		case spv::ExecutionModelTessellationControl:
			m_stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
			break;
		case spv::ExecutionModelTessellationEvaluation:
			m_stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
			break;
		default:
			throw Exception("Unsupported shader stage in shader: " + shaderPath);
	}

	spirv_cross::ShaderResources resources = compiler.get_shader_resources();

	// 1. process uniform buffers
	for (auto& resource : resources.uniform_buffers) {
		uint32_t set =
			compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
		uint32_t binding =
			compiler.get_decoration(resource.id, spv::DecorationBinding);
		const spirv_cross::SPIRType& type = compiler.get_type(resource.type_id);
		uint32_t arraySize = 1;

		if (!type.array.empty())
			arraySize = type.array[0];

		uint32_t size = static_cast<uint32_t>(compiler.get_declared_struct_size(
			compiler.get_type(resource.base_type_id)));

		m_resources.bindings[set].push_back({
			.bindingType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.stages = m_stage,
			.access = VK_ACCESS_UNIFORM_READ_BIT,
			.binding = binding,
			.arraySize = arraySize,
			.size = size,
		});
	}

	// 2. process storage buffers
	for (const auto& resource : resources.storage_buffers) {
		uint32_t set =
			compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
		uint32_t binding =
			compiler.get_decoration(resource.id, spv::DecorationBinding);
		const spirv_cross::SPIRType& type = compiler.get_type(resource.type_id);
		uint32_t arraySize = 1;
		if (!type.array.empty())
			arraySize = type.array[0];
		uint32_t size = static_cast<uint32_t>(compiler.get_declared_struct_size(
			compiler.get_type(resource.base_type_id)));

		BindingInfo bindingInfo{
			.bindingType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.stages = m_stage,
			.access = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
			.binding = binding,
			.arraySize = arraySize,
			.size = size,
		};

		m_resources.bindings[set].push_back(bindingInfo);
	}

	// 3. process image samplers
	for (const auto& resource : resources.sampled_images) {
		uint32_t set =
			compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
		uint32_t binding =
			compiler.get_decoration(resource.id, spv::DecorationBinding);
		const spirv_cross::SPIRType& type = compiler.get_type(resource.type_id);

		uint32_t arraySize = 1;
		if (!type.array.empty())
			arraySize = type.array[0];

		m_resources.bindings[set].push_back({
			.bindingType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.stages = m_stage,
			.access = 0,
			.binding = binding,
			.arraySize = arraySize,
			.size = 0,
		});
	}

	// 4. process push constants
	if (!resources.push_constant_buffers.empty()) {
		const auto& pcResource = resources.push_constant_buffers[0];
		const spirv_cross::SPIRType& type = compiler.get_type(pcResource.type_id);
		uint32_t size =
			static_cast<uint32_t>(compiler.get_declared_struct_size(type));

		m_resources.pushConstants = {
			.stageFlags = m_stage,
			.offset = 0,  // CHECK
			.size = size,
		};
	}
}

Shader::~Shader() {
	vkDestroyShaderModule(m_device.getDevice(), m_module, nullptr);
}

void Shader::getMergedResources(ShaderResources inputResources,
								ShaderResources* outputResources) {
	for (const auto& setPair : inputResources.bindings) {
		uint32_t set = setPair.first;
		const std::vector<BindingInfo>& inputBindings = setPair.second;

		std::vector<BindingInfo>& outputBindings = (*outputResources).bindings[set];

		for (const auto& inBinding : inputBindings) {
			bool found = false;
			for (auto& outBinding : outputBindings) {
				if (outBinding.binding == inBinding.binding &&
					outBinding.bindingType == inBinding.bindingType) {
					outBinding.stages |= inBinding.stages;
					found = true;
					break;
				}

				THROW_ERROR(outBinding.binding == inBinding.binding &&
								outBinding.bindingType != inBinding.bindingType,
							"Incompatible binding types in shader resources");
			}

			if (!found) {
				outputBindings.push_back(inBinding);
			}
		}
	}

	if (inputResources.pushConstants.size == 0) {
		return;
	}

	if ((*outputResources).pushConstants.size == 0) {
		(*outputResources).pushConstants = inputResources.pushConstants;
		return;
	}

	uint32_t outOffset = (*outputResources).pushConstants.offset;
	uint32_t outSize = (*outputResources).pushConstants.size;
	uint32_t inOffset = inputResources.pushConstants.offset;
	uint32_t inSize = inputResources.pushConstants.size;

	uint32_t newOffset = std::min(outOffset, inOffset);
	uint32_t newEnd = std::max(outOffset + outSize, inOffset + inSize);
	(*outputResources).pushConstants.offset = newOffset;
	(*outputResources).pushConstants.size = newEnd - newOffset;
	(*outputResources).pushConstants.stageFlags |=
		inputResources.pushConstants.stageFlags;
}
