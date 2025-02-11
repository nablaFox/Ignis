#include <stdexcept>
#include <vulkan/vulkan_core.h>
#include <vulkan/vk_enum_string_helper.h>

namespace ignis {

class Exception : public std::runtime_error {
public:
	explicit Exception(const std::string& message)
		: std::runtime_error("[Ignis] " + message) {}
};

class VulkanException : public Exception {
public:
	VulkanException(const std::string& message, VkResult result)
		: Exception(message + " (VkResult: " + string_VkResult(result) + ")"),
		  m_result(result) {}

	VkResult result() const noexcept { return m_result; }

private:
	VkResult m_result;
};

#define THROW_VULKAN_ERROR(result, message) \
	if ((result) != VK_SUCCESS)             \
	throw VulkanException(message, result)

#define THROW_ERROR(result, message) \
	if (!(result))                   \
	throw Exception(message)

}  // namespace ignis
