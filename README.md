# Ignis

> The essence of Vulkan

Ignis is a C++ 20 API abstraction over Vulkan.

## Features

1. Ergonomic API in a declarative style.

2. 100% RAII compliant design.

3. Simplicity as a core value in the development.

## Dependencies

- C++ 20 standard library
- Vulkan SDK
- CMake 3.14
- [SPIRV-Cross](https://github.com/KhronosGroup/SPIRV-Cross)
- [VMA](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator)

## Building

Currently, the only officially supported platform is Linux. 

Run:

```sh
cmake -S . -B build
cmake --build build
```

You'll find the compiled library at `build/libignis.a`.

## Usage with CPM.cmake

Add to your `CMakeLists.txt`:

```cmake
CPMAddPackage(
  NAME Ignis
  GITHUB_REPOSITORY nablaFox/Ignis
)

target_link_libraries(YourGameEngine PRIVATE Ignis::Ignis)
```

## Documentation

Work in progress
