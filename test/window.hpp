#include <GLFW/glfw3.h>

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
};
