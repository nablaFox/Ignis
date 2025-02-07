# Ignis

> The essence of Vulkan

A C++ rendering library offering high-level abstractions over the [Vulkan](https://docs.vulkan.org/spec/latest/index.html) API.

## Rationale

This project aims to provide a modern, user-friendly interface to the Vulkan API.

1. Resource management is automatic thanks to a 100% RAII compliant design.

2. The API interface is declarative, with a focus on ease of use and safety.

2. Abstraction, by definition, reduces control and flexibility. The library tries
   to be as minimal as possible while still hiding as many details as it can.

## Building

Currently, the only officially supported platform is Linux. 

First, ensure you have:

- A `C++17` compatible compiler;
- The Vulkan SDK (on Arch Linux: `sudo pacman -S vulkan-devel`);
- CMake 3.14 or later.

Then run:

```sh
cmake -S . -B build
cmake --build build
```

You'll find the library at `build/libignis.a`.

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
