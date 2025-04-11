#include "ignis/shader.hpp"
#include "exceptions.hpp"

#include <fstream>

using namespace ignis;

Shader::Shader(const VkDevice device,
			   const void* code,
			   VkDeviceSize codeSize,
			   VkShaderStageFlagBits stage,
			   VkDeviceSize pushConstantSize)
	: m_device(device), m_pushConstantSize(pushConstantSize), m_stage(stage) {
	THROW_ERROR(codeSize % 4 != 0,
				"SPIR-V shader code size must be a multiple of 4");

	const VkShaderModuleCreateInfo moduleCreateInfo{
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = codeSize,
		.pCode = static_cast<const uint32_t*>(code),
	};

	THROW_VULKAN_ERROR(
		vkCreateShaderModule(m_device, &moduleCreateInfo, nullptr, &m_module),
		"Failed to create shader module");
}

Shader Shader::fromFile(VkDevice device,
						const std::string& path,
						VkShaderStageFlagBits stage,
						VkDeviceSize pushConstantSize) {
	std::ifstream file(path, std::ios::ate | std::ios::binary);

	THROW_ERROR(!file.is_open(), "Failed to open shader file " + path);

	size_t fileSize = static_cast<size_t>(file.tellg());
	std::vector<uint32_t> code(fileSize / sizeof(uint32_t));
	file.seekg(0);
	file.read(reinterpret_cast<char*>(code.data()), fileSize);
	file.close();

	return Shader(device, code.data(), static_cast<VkDeviceSize>(fileSize), stage,
				  pushConstantSize);
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
