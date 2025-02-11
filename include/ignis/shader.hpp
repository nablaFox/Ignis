#pragma once

#include <vulkan/vulkan_core.h>
#include <string>
#include <unordered_map>
#include <vector>

namespace ignis {

class Device;
struct BindingInfo;

struct ShaderResources {
	std::unordered_map<uint32_t, std::vector<BindingInfo>> bindings;
	VkPushConstantRange pushConstants;
};

class Shader {
public:
	Shader(Device&, std::string shaderCode);
	~Shader();

	ShaderResources getResources() const { return m_resources; };

	VkShaderModule getModule() const { return m_module; }

	VkShaderStageFlagBits getStage() const { return m_stage; }

	static void getMergedResources(ShaderResources inputResources,
								   ShaderResources* outputResources);

private:
	Device& m_device;
	ShaderResources m_resources;
	VkShaderStageFlagBits m_stage;
	VkShaderModule m_module;

public:
	Shader(const Shader&) = delete;
	Shader(Shader&&) = delete;
	Shader& operator=(const Shader&) = delete;
	Shader& operator=(Shader&&) = delete;
};

}  // namespace ignis
