#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <array>
#include <iostream>
#include <math.h>
#include "semaphore.hpp"
#include "color_image.hpp"
#include "window.hpp"
#include "buffer.hpp"
#include "device.hpp"
#include "command.hpp"
#include "fence.hpp"
#include "shader.hpp"
#include "pipeline.hpp"
#include "swapchain.hpp"

using namespace ignis;

#define PRINT(x) std::cout << #x << ": " << (x) << std::endl

constexpr size_t WINDOW_WIDTH = 800;
constexpr size_t WINDOW_HEIGHT = 600;

// 8 bit per channel
struct Pixel {
	__bf16 r;
	__bf16 g;
	__bf16 b;
	__bf16 a;
};

typedef std::array<float, 2> vec2;

struct Screen {
	std::array<Pixel, WINDOW_WIDTH * WINDOW_HEIGHT> pixels;

	Pixel& pixelAt(int x, int y) {
		if (x < 0 || x >= WINDOW_WIDTH || y < 0 || y >= WINDOW_HEIGHT) {
			return pixels[0];
		}

		return pixels[x + y * WINDOW_WIDTH];
	}

	Pixel& pixelAt(vec2 pos) { return pixelAt(pos[0], pos[1]); }

	void clearScreen() {
		for (int i = 0; i < WINDOW_WIDTH * WINDOW_HEIGHT; i++) {
			pixels[i] = Pixel{.r = 0, .g = 0, .b = 0, .a = 0};
		}
	}

	void rotate(vec2& position, float angle) {
		float x = position[0];
		float y = position[1];

		position[0] = x * cos(angle) - y * sin(angle);
		position[1] = x * sin(angle) + y * cos(angle);
	}

	void drawLine(vec2 start, vec2 end, Pixel color) {
		float dx = powf(end[0] - start[0], 2);
		float dy = powf(end[1] - start[1], 2);
		float L = sqrt(dx + dy);

		vec2 dir = {end[0] - start[0], end[1] - start[1]};
		dir[0] /= L;
		dir[1] /= L;

		for (int t = 0; t < (int)L; t++) {
			pixelAt({start[0] + dir[0] * t, start[1] + dir[1] * t}) = color;
		}
	}

	void drawRotatedSquare(float angle, vec2 pos, float sideLength) {
		Pixel color{
			.r = 0,
			.g = UINT8_MAX,
			.b = 0,
			.a = UINT8_MAX,
		};

		clearScreen();

		vec2 offsetBR = {sideLength / 2.0f, sideLength / 2.0f};
		vec2 offsetBL = {-sideLength / 2.0f, sideLength / 2.0f};
		vec2 offsetUL = {-sideLength / 2.0f, -sideLength / 2.0f};
		vec2 offsetUR = {sideLength / 2.0f, -sideLength / 2.0f};

		rotate(offsetBR, angle);
		rotate(offsetBL, angle);
		rotate(offsetUL, angle);
		rotate(offsetUR, angle);

		vec2 bottomRight = {pos[0] + offsetBR[0], pos[1] + offsetBR[1]};
		vec2 bottomLeft = {pos[0] + offsetBL[0], pos[1] + offsetBL[1]};
		vec2 upLeft = {pos[0] + offsetUL[0], pos[1] + offsetUL[1]};
		vec2 upRight = {pos[0] + offsetUR[0], pos[1] + offsetUR[1]};

		drawLine(bottomRight, bottomLeft, color);
		drawLine(bottomLeft, upLeft, color);
		drawLine(upLeft, upRight, color);
		drawLine(upRight, bottomRight, color);
	}
};

int main(int argc, char* argv[]) {
	Window window(WINDOW_WIDTH, WINDOW_HEIGHT);

	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions =
		glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> instanceExtensions;
	for (uint32_t i = 0; i < glfwExtensionCount; i++) {
		instanceExtensions.push_back(glfwExtensions[i]);
	}

	Device device({
		.shadersFolder = "test/shaders",
		.extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME},
		.instanceExtensions = instanceExtensions,
	});

	VkSurfaceKHR surface = nullptr;
	if (glfwCreateWindowSurface(device.getInstance(), window.handle, nullptr,
								&surface) != VK_SUCCESS) {
		PRINT("errror creating the surface");
		return 1;
	}

	Swapchain swapchain({
		.device = &device,
		.extent = {WINDOW_WIDTH, WINDOW_HEIGHT},
		.surface = surface,
		.presentMode = VK_PRESENT_MODE_FIFO_KHR,
	});

	ColorImage* drawImage =
		ColorImage::createDrawImage(&device, {WINDOW_WIDTH, WINDOW_HEIGHT});

	Screen screen{};

	Command updatePixelsCmd(device);
	Fence waitForRendering(device, true);
	Semaphore finishedRendering(device);

	float angle = 0.f;
	const float angularVel = 0.01f;

	vec2 squareOffset = {0, 0};

	while (!window.shouldWindowClose()) {
		angle += angularVel;

		if (glfwGetKey(window.handle, GLFW_KEY_W) == GLFW_PRESS) {
			squareOffset[1] -= 1;
		} else if (glfwGetKey(window.handle, GLFW_KEY_S) == GLFW_PRESS) {
			squareOffset[1] += 1;
		} else if (glfwGetKey(window.handle, GLFW_KEY_A) == GLFW_PRESS) {
			squareOffset[0] -= 1;
		} else if (glfwGetKey(window.handle, GLFW_KEY_D) == GLFW_PRESS) {
			squareOffset[0] += 1;
		}

		screen.drawRotatedSquare(angle,
								 {(float)WINDOW_WIDTH / 2 + squareOffset[0],
								  (float)WINDOW_HEIGHT / 2 + squareOffset[1]},
								 100);

		waitForRendering.wait();
		waitForRendering.reset();

		updatePixelsCmd.begin();
		updatePixelsCmd.transitionImageLayout(*drawImage,
											  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		updatePixelsCmd.updateImage(*drawImage, screen.pixels.data());

		updatePixelsCmd.transitionToOptimalLayout(*drawImage);

		updatePixelsCmd.end();

		SubmitCmdInfo submitUpdatePixelsInfo{
			.command = &updatePixelsCmd,
			.signalSemaphores = {&finishedRendering},
		};

		device.submitCommands({std::move(submitUpdatePixelsInfo)}, waitForRendering);

		swapchain.present({
			.image = drawImage,
			.waitSemaphores = {&finishedRendering},
		});
	}

	delete drawImage;

	return 0;
}
