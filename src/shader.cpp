#include "ignis/shader.hpp"
#include "ignis/exceptions.hpp"

#include <fstream>

using namespace ignis;

Shader::Shader(const VkDevice device,
			   const std::string& shaderPath,
			   VkShaderStageFlagBits stage,
			   VkDeviceSize pushConstantSize)
	: m_device(device), m_pushConstantSize(pushConstantSize), m_stage(stage) {
	std::ifstream file(shaderPath, std::ios::ate | std::ios::binary);

	THROW_ERROR(!file.is_open(), "Failed to open shader file " + shaderPath);

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

	THROW_VULKAN_ERROR(
		vkCreateShaderModule(m_device, &moduleCreateInfo, nullptr, &m_module),
		"Failed to create shader module");
}

Shader::~Shader() {
	vkDestroyShaderModule(m_device, m_module, nullptr);
}

uint32_t Shader::getMergedPushConstantSize(const std::vector<Shader*>& shaders) {
	uint32_t mergedSize{0};

	for (const auto* shader : shaders) {
		mergedSize = std::max(mergedSize, shader->getPushConstantSize());
	}

	return mergedSize;
}
