#include "buffer.hpp"
#include "depth_image.hpp"
#include "device.hpp"
#include "color_image.hpp"
#include "command.hpp"
#include "fence.hpp"
#include "shader.hpp"
#include "pipeline.hpp"

using namespace ignis;

void printBindingInfo(const BindingInfo& info) {
	printf("Binding %d: type %d, stages %d, access %d, array size %d, size %d\n",
		   info.binding, info.bindingType, info.stages, info.access, info.arraySize,
		   info.size);
}

void printShaderResources(const ShaderResources& resources) {
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

int main(int argc, char* argv[]) {
	Device device({
		.shadersFolder = "test/shaders",
		.extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME},
		.instanceExtensions = {VK_KHR_SURFACE_EXTENSION_NAME},
	});

	struct Color {
		uint32_t r;
		uint32_t g;
		uint32_t b;
		uint32_t a;
	};

	Color* pixels = new Color[800 * 600];

	for (uint32_t i = 0; i < 800 * 600; i++) {
		pixels[i] = {0, 0, 0, 0};
	}

	auto drawImage = ColorImage::createDrawImage(&device, {800, 600});
	auto depthImage = DepthImage::createDepthStencilImage(&device, {800, 600});

	Buffer* ubo = Buffer::createUBO<Color>(&device, 1, pixels);

	Buffer testBuffer({
		.device = &device,
		.bufferUsage = VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		.elementSize = sizeof(Color),
		.elementCount = 800 * 600,
		.stride = sizeof(Color),
	});

	Shader shader(device, "vertex.spv");

	printShaderResources(shader.getResources());

	Pipeline pipeline({
		.device = &device,
		.shaders = {"vertex.spv"},
		.colorFormat = drawImage->getFormat(),
		.depthFormat = depthImage->getFormat(),
	});

	Command cmd(device);

	cmd.begin();

	cmd.updateImage(*drawImage, pixels);

	cmd.updateBuffer(testBuffer, pixels);

	cmd.bindPipeline(pipeline);

	cmd.bindUBO(*ubo, 0, 1);

	cmd.end();

	Fence fence(device);

	device.submitCommands({{&cmd}}, fence);

	fence.wait();

	delete drawImage;
	delete depthImage;
	delete ubo;

	return 0;
}
