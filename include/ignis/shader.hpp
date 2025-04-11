#pragma once

#include <vulkan/vulkan_core.h>
#include <string>
#include <vector>

namespace ignis {

class Shader {
public:
	Shader(const VkDevice,
		   const void*,
		   VkDeviceSize codeSize,
		   VkShaderStageFlagBits,
		   VkDeviceSize pushConstantsSize = 0);

	static Shader fromFile(VkDevice device,
						   const std::string& path,
						   VkShaderStageFlagBits stage,
						   VkDeviceSize pushConstantSize = 0);

	~Shader();

	auto getModule() const { return m_module; }

	auto getStage() const { return m_stage; }

	auto getPushConstantSize() const { return m_pushConstantSize; }

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
