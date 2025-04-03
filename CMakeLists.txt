cmake_minimum_required(VERSION 3.14 FATAL_ERROR)
project(ignis)

include(cmake/CPM.cmake)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(IGNIS_BUILD_EXAMPLES "Build the examples" ON)
option(IGNIS_INSTALL "Install the library" OFF)

find_package(Vulkan REQUIRED)

file(GLOB IGNIS_SRC "src/*.cpp")
add_library(ignis STATIC ${IGNIS_SRC})
target_include_directories(ignis PUBLIC include/ignis)

add_library(Ignis::Ignis ALIAS ignis)

CPMAddPackage(
  NAME VulkanMemoryAllocator
  GITHUB_REPOSITORY GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
  GIT_TAG v3.2.0
)

CPMAddPackage(
  NAME SPIRV-Cross
  GITHUB_REPOSITORY KhronosGroup/SPIRV-Cross
  GIT_TAG 2021-01-15
  OPTIONS
    "SPIRV_CROSS_CLI OFF"
    "SPIRV_CROSS_ENABLE_TESTS OFF"
    "SPIRV_CROSS_ENABLE_HLSL OFF"
    "SPIRV_CROSS_ENABLE_MSL OFF"
	"SPIRV_CROSS_ENABLE_C_API OFF"
    "SPIRV_CROSS_ENABLE_REFLECT OFF"
    "SPIRV_CROSS_ENABLE_UTIL OFF"
	"SPIRV_CROSS_SKIP_INSTALL ON"
)

target_link_libraries(ignis PUBLIC
  Vulkan::Vulkan
  GPUOpen::VulkanMemoryAllocator
  spirv-cross-cpp
)

if (IGNIS_INSTALL)
	install(TARGETS ignis 
		ARCHIVE DESTINATION lib
		LIBRARY DESTINATION lib
		INCLUDES DESTINATION include
	)
endif()

if (IGNIS_BUILD_EXAMPLES)
	find_package(glfw3 REQUIRED)

	file(GLOB EXAMPLES_SRC "examples/*.cpp")

	foreach(EXAMPLE_SRC ${EXAMPLES_SRC})
	  get_filename_component(EXAMPLE_NAME ${EXAMPLE_SRC} NAME_WE)
	  add_executable(${EXAMPLE_NAME} ${EXAMPLE_SRC})
	  target_link_libraries(${EXAMPLE_NAME} PRIVATE Ignis::Ignis glfw)
	endforeach()
endif()
