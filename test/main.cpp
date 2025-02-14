#include "device.hpp"
#include "color_image.hpp"
#include "command.hpp"
#include "fence.hpp"

using namespace ignis;

int main(int argc, char* argv[]) {
	Device device({});

	struct Color {
		uint16_t r;
		uint16_t g;
		uint16_t b;
		uint16_t a;
	};

	Color* pixels = new Color[800 * 600];

	for (uint32_t i = 0; i < 800 * 600; i++) {
		pixels[i] = {0, 0, 0, 0};
	}

	auto drawImage = ColorImage::createDrawImage(&device, {800, 600});

	Command cmd(device);

	cmd.begin();

	cmd.updateImage(*drawImage, pixels);

	cmd.transitionImageLayout(*drawImage, drawImage->getOptimalLayout());

	cmd.end();

	Fence fence(device);

	device.submitCommands({{&cmd}}, fence);

	fence.wait();

	delete drawImage;

	return 0;
}
