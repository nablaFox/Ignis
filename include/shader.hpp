#pragma once

#include <vulkan/vulkan_core.h>
#include <string>
#include <unordered_map>
#include <vector>

namespace ignis {

class Device;
class Binding;

struct ShaderResources {
	std::unordered_map<uint32_t, std::vector<Binding>> bindings;
	VkPushConstantRange pushConstants;
};

class Shader {
public:
	Shader(Device&, std::string shaderCode);
	~Shader();

	ShaderResources getResources() const { return m_resources; };

	VkShaderModule getModule() const { return m_module; }

	VkShaderStageFlagBits getStage() const { return m_stage; }

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
