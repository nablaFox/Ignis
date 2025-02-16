cmake_minimum_required(VERSION 3.14 FATAL_ERROR)
project(ignis)

include(cmake/CPM.cmake)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Vulkan REQUIRED)

file(GLOB IGNIS_SRC "src/*.cpp")
add_library(ignis STATIC ${IGNIS_SRC})
target_include_directories(ignis PUBLIC include/ignis)

add_library(Ignis::Ignis ALIAS ignis)

# TEMP
CPMAddPackage(
  NAME VulkanMemoryAllocator
  GITHUB_REPOSITORY GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
  GIT_TAG v3.2.0
)

# TEMP
CPMAddPackage(
  NAME SPIRV-Cross
  GITHUB_REPOSITORY KhronosGroup/SPIRV-Cross
  GIT_TAG 2021-01-15
)

target_include_directories(ignis PUBLIC
	SPIRV-CRoss_SOURCE_DIR
)

target_link_libraries(ignis PUBLIC
  Vulkan::Vulkan
  GPUOpen::VulkanMemoryAllocator
  spirv-cross-cpp
)

# "tests"
option(IGNIS_BUILD_TESTS "Build the tests" ON)

if(NOT IGNIS_BUILD_TESTS)
  return()
endif()

find_package(glfw3 REQUIRED)

add_executable(ignis_test test/main.cpp)
target_link_libraries(ignis_test PRIVATE Ignis::Ignis glfw)
