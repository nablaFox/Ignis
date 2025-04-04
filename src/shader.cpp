#include "shader.hpp"
#include "exceptions.hpp"

#include <fstream>

using namespace ignis;

Shader::Shader(const ShaderCreateInfo& info)
	: m_device(info.device),
	  m_pushConstantSize(info.pushConstantSize),
	  m_stage(info.stage) {
	std::ifstream file(info.shaderPath, std::ios::ate | std::ios::binary);

	THROW_ERROR(!file.is_open(), "Failed to open shader file " + info.shaderPath);

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
