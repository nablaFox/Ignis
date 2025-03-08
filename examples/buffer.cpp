#include "buffer.hpp"
#include "device.hpp"
#include "utils.hpp"

struct TestData {
	float scale;
	vec2 position;
};

int main(int argc, char* argv[]) {
	Device device({});

	Buffer testUbo = device.createUBO(sizeof(TestData));

	print(testUbo.getSize());
	print(sizeof(TestData));

	TestData data{1.0f, {2.0f, 3.0f}};
	testUbo.writeData(&data);

	TestData readData{};
	testUbo.readData(&readData);

	print(readData.scale);

	return 0;
}
