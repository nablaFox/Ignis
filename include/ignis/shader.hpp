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

// Note 1: we don't support supplying the entry points
// Note 2: we don't support supplying the shader code directly

class Shader {
public:
	Shader(const Device&, std::string shaderPath);
	~Shader();

	ShaderResources getResources() const { return m_resources; };

	VkShaderModule getModule() const { return m_module; }

	VkShaderStageFlagBits getStage() const { return m_stage; }

	static void getMergedResources(ShaderResources inputResources,
								   ShaderResources* outputResources);

private:
	const Device& m_device;
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
