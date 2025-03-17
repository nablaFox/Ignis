#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <math.h>
#include "semaphore.hpp"
#include "device.hpp"
#include "command.hpp"
#include "fence.hpp"
#include "shader.hpp"
#include "pipeline.hpp"
#include "swapchain.hpp"
#include "utils.hpp"

using namespace ignis;

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
	float precision = 100.f;

	void draw(Screen& screen,
			  vec2 pos,
			  float scaleFactor = 0,
			  float rot = 0) override {
		if (precision < 2)
			precision = 2;

		int precisionInt = std::ceil(precision);

		vertices.resize(precisionInt);

		for (int i = 0; i < precisionInt; i++) {
			float angle = (float)i * 2 * (float)M_PI / (float)precisionInt;
			vertices[i] = {{cosf(angle), sinf(angle)}, color};
		}

		vertices.push_back({cosf(0), sinf(0)});

		scale(vertices, scaleFactor);
		rotate(vertices, rot);
		translate(vertices, pos);
		screen.drawVertices(vertices);
	}
};

struct PhysicalObject {
	constexpr static int pixelsPerMeter = 100;
	constexpr static vec2 gravity = {0.f, 5.f};

	float mass;
	vec2 force;
	vec2 acc;
	vec2 vel;
	vec2 pos;

	float shapeScale;

	void update(float dt) {
		vec2 netForce = (gravity * mass) + force;
		netForce *= pixelsPerMeter;
		acc = netForce / mass;
		vel += acc * dt;
		pos += vel * dt;
	}

	void draw(Screen& screen) { shape->draw(screen, pos, shapeScale); }

	Shape* shape;
};

struct PhysicalSquare : PhysicalObject {
	PhysicalSquare(float mass,
				   vec2 initialPos,
				   float shapeScale = 10.f,
				   vec2 initialVel = {},
				   vec2 initialForce = {})
		: PhysicalObject{mass,		 initialForce, {},
						 initialVel, initialPos,   shapeScale} {
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
		return 1;
	}

	Swapchain swapchain({
		.device = &device,
		.extent = {WINDOW_WIDTH, WINDOW_HEIGHT},
		.format = ColorFormat::RGBA8,
		.surface = surface,
	});

	Image drawImage = device.createDrawAttachmentImage({
		.width = WINDOW_WIDTH,
		.height = WINDOW_HEIGHT,
		.sampleCount = VK_SAMPLE_COUNT_1_BIT,
	});

	Screen screen{};

	PhysicalSquare square(10.f, {CENTER_X - 50, 100}, 50.f);
	PhysicalSquare square2(100.f, {CENTER_X + 50, 100}, 100.f);

	vec2 pushIntensity = {8.f * square.mass, 9.8f * square.mass};

	Command updatePixelsCmd({.device = device});
	Command blitCmd({.device = device});
	Fence waitForRendering(device, true);
	Semaphore finishedRendering(device);
	Semaphore finishedBlit(device);
	Semaphore acquiredImage(device);

	const float dt = 1.f / 60.f;
	float timeAccumulator = 0.f;

	while (!window.shouldWindowClose()) {
		screen.clearScreen();

		timeAccumulator += window.getFrameTime();

		vec2 pushForce = {0, 0};

		// check if space is pressed
		if (glfwGetKey(window.handle, GLFW_KEY_SPACE) == GLFW_PRESS) {
			pushForce[1] = -pushIntensity[1];
		}

		if (glfwGetKey(window.handle, GLFW_KEY_A) == GLFW_PRESS) {
			pushForce[0] = -pushIntensity[0];
		}

		if (glfwGetKey(window.handle, GLFW_KEY_D) == GLFW_PRESS) {
			pushForce[0] = pushIntensity[0];
		}

		if (glfwGetKey(window.handle, GLFW_KEY_UP) == GLFW_PRESS) {
			pushIntensity[1] += 0.07f;
		}

		if (glfwGetKey(window.handle, GLFW_KEY_DOWN) == GLFW_PRESS) {
			pushIntensity[1] -= 0.07f;
		}

		square.force = pushForce;

		while (timeAccumulator >= dt) {
			square.update(dt);
			square2.update(dt);
			timeAccumulator -= dt;
		}

		square.draw(screen);
		square2.draw(screen);

		waitForRendering.wait();
		waitForRendering.reset();

		updatePixelsCmd.begin();
		updatePixelsCmd.transitionImageLayout(drawImage,
											  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		updatePixelsCmd.updateImage(drawImage, screen.pixels.data());

		updatePixelsCmd.transitionToOptimalLayout(drawImage);

		updatePixelsCmd.end();

		SubmitCmdInfo submitUpdatePixelsInfo{
			.command = updatePixelsCmd,
			.signalSemaphores = {&finishedRendering},
		};

		device.submitCommands({std::move(submitUpdatePixelsInfo)}, nullptr);

		Image& swapchainImage = swapchain.acquireNextImage(&acquiredImage);

		blitCmd.begin();

		blitCmd.transitionImageLayout(drawImage,
									  VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

		blitCmd.transitionImageLayout(swapchainImage,
									  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		blitCmd.blitImage(drawImage, swapchainImage);

		blitCmd.transitionToOptimalLayout(swapchainImage);
		blitCmd.transitionToOptimalLayout(drawImage);

		blitCmd.end();

		SubmitCmdInfo submitBlitInfo{
			.command = blitCmd,
			.waitSemaphores = {&acquiredImage, &finishedRendering},
			.signalSemaphores = {&finishedBlit},
		};

		device.submitCommands({std::move(submitBlitInfo)}, &waitForRendering);

		swapchain.presentCurrent({
			.waitSemaphores = {&finishedBlit},
		});
	}

	waitForRendering.wait();

	return 0;
}
