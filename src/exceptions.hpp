#pragma once

#include <stdexcept>
#include <sstream>
#include <vulkan/vulkan_core.h>
#include <vulkan/vk_enum_string_helper.h>

namespace ignis {

class Exception : public std::runtime_error {
public:
	Exception(const std::string& message,
			  const char* file,
			  int line,
			  const char* function)
		: std::runtime_error(formatMessage(message, file, line, function)),
		  m_file(file),
		  m_line(line),
		  m_function(function) {}

	const char* file() const noexcept { return m_file; }
	int line() const noexcept { return m_line; }
	const char* function() const noexcept { return m_function; }

private:
	static std::string formatMessage(const std::string& message,
									 const char* file,
									 int line,
									 const char* function) {
		std::ostringstream oss;
		oss << "[Ignis] " << file << ":" << line << " (" << function << ") - "
			<< message;
		return oss.str();
	}

	const char* m_file;
	int m_line;
	const char* m_function;
};

class VulkanException : public Exception {
public:
	VulkanException(const std::string& message,
					VkResult result,
					const char* file,
					int line,
					const char* function)
		: Exception(message + " [VkResult: " + string_VkResult(result) + "]",
					file,
					line,
					function),
		  m_result(result) {}

	VkResult result() const noexcept { return m_result; }

private:
	VkResult m_result;
};

#define THROW_VULKAN_ERROR(result, message) \
	if ((result) != VK_SUCCESS)             \
	throw ignis::VulkanException((message), (result), __FILE__, __LINE__, __func__)

#define THROW_ERROR(condition, message) \
	if (condition)                      \
	throw ignis::Exception((message), __FILE__, __LINE__, __func__)

}  // namespace ignis
