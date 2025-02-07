#pragma once

#include <vulkan/vulkan_core.h>
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
	Shader(Device&);
	~Shader();

	ShaderResources getResources() { return m_resources; };

private:
	Device& m_device;
	VkShaderModule m_module;
	ShaderResources m_resources;

public:
	Shader(const Shader&) = delete;
	Shader(Shader&&) = delete;
	Shader& operator=(const Shader&) = delete;
	Shader& operator=(Shader&&) = delete;
};

}  // namespace ignis
