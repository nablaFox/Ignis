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

	Buffer* testUbo = Buffer::createUBO(&device, sizeof(TestData));

	print(testUbo->getSize());
	print(sizeof(TestData));

	TestData data{1.0f, {2.0f, 3.0f}};
	testUbo->writeData(&data);

	TestData readData{};
	testUbo->readData(&readData);

	print(readData.scale);

	delete testUbo;

	Buffer* emptyBuffer = Buffer::createUBO(&device, 0);

	delete emptyBuffer;

	return 0;
}
