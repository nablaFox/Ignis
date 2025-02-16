#include "buffer.hpp"
#include "device.hpp"
#include "math.hpp"
#include "utils.hpp"

struct TestData {
	float scale;
	vec2 position;
};

using namespace ignis;

int main(int argc, char* argv[]) {
	Device device({});

	Buffer* testUbo = Buffer::createUBO<TestData>(&device, 1);

	print(testUbo->getSize());
	print(testUbo->getElementCount());
	print(testUbo->getElementSize());
	print(testUbo->getStride());

	TestData data{1.0f, {2.0f, 3.0f}};
	testUbo->writeData(&data);

	delete testUbo;

	return 0;
}
