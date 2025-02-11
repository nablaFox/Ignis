#include <vector>
#include <vulkan/vulkan.h>

namespace ignis {

class VertexInputLayout {
public:
	VertexInputLayout& addBinding(
		uint32_t binding,
		uint32_t stride,
		VkVertexInputRate inputRate = VK_VERTEX_INPUT_RATE_VERTEX);

	VertexInputLayout& addAttribute(uint32_t location,
									uint32_t binding,
									VkFormat format,
									uint32_t offset);

	const std::vector<VkVertexInputBindingDescription>& getBindings() const {
		return m_bindings;
	}

	const std::vector<VkVertexInputAttributeDescription>& getAttributes() const {
		return m_attributes;
	}

private:
	std::vector<VkVertexInputBindingDescription> m_bindings;
	std::vector<VkVertexInputAttributeDescription> m_attributes;
};

}  // namespace ignis
