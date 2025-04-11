cmake_minimum_required(VERSION 3.14 FATAL_ERROR)
project(Ignis)

include(cmake/CPM.cmake)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB IGNIS_SRC "src/*.cpp")
add_library(Ignis STATIC ${IGNIS_SRC})

target_include_directories(Ignis PUBLIC 
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

find_package(Vulkan REQUIRED)

CPMAddPackage(
  NAME VulkanMemoryAllocator
  GITHUB_REPOSITORY GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
  GIT_TAG v3.2.0
)

target_link_libraries(Ignis PUBLIC
  Vulkan::Vulkan
  GPUOpen::VulkanMemoryAllocator
)
