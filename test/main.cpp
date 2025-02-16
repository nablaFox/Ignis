#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "math.hpp"
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

#define PRINT(x) std::cout << (x) << std::endl

constexpr size_t WINDOW_WIDTH = 800;
constexpr size_t WINDOW_HEIGHT = 600;
const float CENTER_X = (float)WINDOW_WIDTH / 2;
const float CENTER_Y = (float)WINDOW_HEIGHT / 2;
const vec2 CENTER = {CENTER_X, CENTER_Y};

// 8 bit per channel
struct Pixel {
	__bf16 r;
	__bf16 g;
	__bf16 b;
	__bf16 a;
};

constexpr Pixel RED{.r = UINT8_MAX};
constexpr Pixel BLUE{.b = UINT8_MAX};
constexpr Pixel GREEN{.g = UINT8_MAX};

struct Vertex {
	vec2 pos;
	Pixel color;
};

typedef uint32_t Index;

struct Screen {
	std::array<Pixel, WINDOW_WIDTH * WINDOW_HEIGHT> pixels;

	void drawPixel(int x, int y, Pixel color) {
		if (x < 0 || x >= WINDOW_WIDTH || y < 0 || y >= WINDOW_HEIGHT) {
			return;
		}

		pixels[x + y * WINDOW_WIDTH] = color;
	}

	void drawPixel(vec2 pos, Pixel color) { drawPixel(pos[0], pos[1], color); }

	void clearScreen() {
		for (int i = 0; i < WINDOW_WIDTH * WINDOW_HEIGHT; i++) {
			pixels[i] = Pixel{};
		}
	}

	void drawLine(Vertex start, Vertex end) {
		float dx = powf(end.pos[0] - start.pos[0], 2);
		float dy = powf(end.pos[1] - start.pos[1], 2);
		float L = sqrt(dx + dy);

		vec2 dir = end.pos - start.pos;

		dir /= L;

		for (int t = 0; t < (int)L; t++) {
			drawPixel(dir * (float)t + start.pos, start.color);
		}
	}

	void drawVertices(const std::vector<Vertex>& vertices) {
		for (int i = 0; i < vertices.size() - 1; i++) {
			drawLine(vertices[i], vertices[i + 1]);
		}
	}

	void drawVertices(const std::vector<Vertex>& vertices,
					  const std::vector<Index>& indices) {
		for (int i = 0; i < indices.size(); i += 2) {
			drawLine(vertices[indices[i]], vertices[indices[i + 1]]);
		}
	}
};

struct Shape {
	std::vector<Vertex> vertices;
	std::vector<Index> indices;

	Pixel color;

	void rotate(std::vector<Vertex>& vertices, float angle) {
		for (auto& vertex : vertices) {
			float x = vertex.pos[0];
			float y = vertex.pos[1];

			vertex.pos[0] = x * cos(angle) - y * sin(angle);
			vertex.pos[1] = x * sin(angle) + y * cos(angle);
		}
	}

	void translate(std::vector<Vertex>& vertices, vec2 offset) {
		for (auto& vertex : vertices) {
			vertex.pos += offset;
		}
	}

	void scale(std::vector<Vertex>& vertices, float scaleFactor) {
		for (auto& vertex : vertices) {
			vertex.pos *= scaleFactor;
		}
	}

	virtual void draw(Screen& screen,
					  vec2 pos,
					  float scaleFactor = 1,
					  float rot = 0) {
		rotate(vertices, rot);
		scale(vertices, scaleFactor);
		translate(vertices, pos);

		screen.drawVertices(vertices, indices);
	}
};

struct Square : Shape {
	void draw(Screen& screen,
			  vec2 pos,
			  float scaleFactor = 1,
			  float rot = 0) override {
		vertices = {
			{1.f / 2.f, 1.f / 2.f, color},	  // bottom right
			{-1.f / 2.f, 1.f / 2.f, color},	  // bottom left
			{-1.f / 2.f, -1.f / 2.f, color},  // up left
			{1.f / 2.f, -1.f / 2.f, color},	  // up right
		};

		indices = {
			0, 1,  // bottom line
			1, 2,  // up left line
			2, 3,  // up line
			3, 0,  // up right line
		};

		Shape::draw(screen, pos, scaleFactor, rot);
	}
};

struct Circle : Shape {
	void draw(Screen& screen,
			  vec2 pos,
			  float scaleFactor = 0,
			  float rot = 1) override {
		constexpr int precision = 110;

		vertices.resize(precision);

		for (int i = 0; i < precision; i++) {
			float angle = (float)i * 2 * (float)M_PI / precision;
			vertices[i] = {{cosf(angle), sinf(angle)}, color};
		}

		vertices.push_back({cosf(0), sinf(0)});

		scale(vertices, scaleFactor);
		translate(vertices, pos);
		screen.drawVertices(vertices);
	}
};

struct PhysicalObject {
	constexpr static float shapeScale = 100;

	float mass;
	vec2 force;
	vec2 acc;
	vec2 vel;
	vec2 pos;

	void update(float dt) {
		acc = force / mass;
		vel += acc * dt;
		pos += vel * dt;
	}

	void applyForce(vec2 f) { force += f; }

	void draw(Screen& screen) { shape->draw(screen, pos, shapeScale); }

	Shape* shape;
};

struct PhysicalSquare : PhysicalObject {
	PhysicalSquare(float mass,
				   vec2 initialPos,
				   vec2 initialVel = {},
				   vec2 initialForce = {})
		: PhysicalObject{mass, initialForce, {}, initialVel, initialPos} {
		shape = new Square();
		shape->color = RED;
	}

	// for a square the shapeScale is the side length
	// and we have no rotations
	void update(float dt) {
		const float damping = 0.6f;
		const float halfSize = PhysicalObject::shapeScale / 2.0f;

		PhysicalObject::update(dt);

		if (pos[0] < halfSize) {
			pos[0] = halfSize;
			vel[0] = -vel[0] * damping;
		} else if (pos[0] > WINDOW_WIDTH - halfSize) {
			pos[0] = WINDOW_WIDTH - halfSize;
			vel[0] = -vel[0] * damping;
		}

		if (pos[1] < halfSize) {
			pos[1] = halfSize;
			vel[1] = -vel[1] * damping;
		} else if (pos[1] > WINDOW_HEIGHT - halfSize) {
			pos[1] = WINDOW_HEIGHT - halfSize;
			vel[1] = -vel[1] * damping;
		}
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

	const vec2 gravity = {0.f, 10.0f};
	const float simulationTime = 5.f;

	PhysicalSquare square(10.f, CENTER);

	const float pushIntensity = 10.f * square.mass;

	Command updatePixelsCmd(device);
	Fence waitForRendering(device, true);
	Semaphore finishedRendering(device);

	while (!window.shouldWindowClose()) {
		screen.clearScreen();

		float dt = (float)window.getFrameTime();

		square.force = gravity * square.mass;

		// check if space is pressed
		if (glfwGetKey(window.handle, GLFW_KEY_SPACE) == GLFW_PRESS) {
			square.force = {0, -pushIntensity};
		}

		if (glfwGetKey(window.handle, GLFW_KEY_LEFT) == GLFW_PRESS) {
			square.force = {-pushIntensity, 0};
		}

		if (glfwGetKey(window.handle, GLFW_KEY_RIGHT) == GLFW_PRESS) {
			square.force = {pushIntensity, 0};
		}

		square.update(dt * simulationTime);
		square.draw(screen);

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
