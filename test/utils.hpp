#include "shader.hpp"

using namespace ignis;

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
