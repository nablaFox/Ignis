#pragma once

#include <vulkan/vulkan_core.h>
#include <string>
#include <vector>

namespace ignis {

class Shader {
public:
	Shader(const VkDevice,
		   const std::string& shaderPath,
		   VkShaderStageFlagBits,
		   VkDeviceSize pushConstantsSize);

	~Shader();

	VkShaderModule getModule() const { return m_module; }

	VkShaderStageFlagBits getStage() const { return m_stage; }

	uint32_t getPushConstantSize() const { return m_pushConstantSize; }

	static uint32_t getMergedPushConstantSize(const std::vector<Shader*>&);

private:
	const VkDevice m_device;
	VkShaderModule m_module{nullptr};
	uint32_t m_pushConstantSize;
	VkShaderStageFlagBits m_stage;

public:
	Shader(const Shader&) = delete;
	Shader(Shader&&) = delete;
	Shader& operator=(const Shader&) = delete;
	Shader& operator=(Shader&&) = delete;
};

}  // namespace ignis
