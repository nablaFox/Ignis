#include <GLFW/glfw3.h>
#include <iostream>
#include <array>

#include "shader.hpp"

using namespace ignis;

template <typename T>
constexpr inline void print(T&& t) {
	std::cout << t << std::endl;
}

inline void printBindingInfo(const BindingInfo& info) {
	printf("Binding %d: type %d, stages %d, access %d, array size %d, size %d\n",
		   info.binding, info.bindingType, info.stages, info.access, info.arraySize,
		   info.size);
}

inline void printShaderResources(const ShaderResources& resources) {
	printf("Push constants: stages %d, offset %d, size %d\n",
		   resources.pushConstants.stageFlags, resources.pushConstants.offset,
		   resources.pushConstants.size);
	for (const auto& [set, bindings] : resources.bindings) {
		printf("Set %d:\n", set);
		for (const auto& binding : bindings) {
			printBindingInfo(binding);
		}
	}
}

class Window {
public:
	Window(int width, int height) {
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		handle = glfwCreateWindow(width, height, "Test", nullptr, nullptr);
	}

	GLFWwindow* handle;

	bool shouldWindowClose() {
		glfwPollEvents();
		return glfwWindowShouldClose(handle);
	}

	float getFrameTime() {
		float currentTime = (float)glfwGetTime();
		static float lastTime = 0;

		float frameTime = currentTime - lastTime;
		lastTime = currentTime;

		return frameTime;
	}
};

typedef std::array<float, 2> vec2;

vec2 operator+(const vec2& a, const vec2& b) {
	return {a[0] + b[0], a[1] + b[1]};
}

vec2 operator+=(vec2& a, const vec2& b) {
	a[0] += b[0];
	a[1] += b[1];
	return a;
}

vec2 operator-(const vec2& a, const vec2& b) {
	return {a[0] - b[0], a[1] - b[1]};
}

vec2 operator*(const vec2& a, float b) {
	return {a[0] * b, a[1] * b};
}

vec2 operator*=(vec2& a, float b) {
	a[0] *= b;
	a[1] *= b;
	return a;
}

vec2 operator/=(vec2& a, float b) {
	a[0] /= b;
	a[1] /= b;
	return a;
}

vec2 operator/(const vec2& a, float b) {
	return {a[0] / b, a[1] / b};
}
