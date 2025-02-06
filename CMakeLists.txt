cmake_minimum_required(VERSION 3.14 FATAL_ERROR)
project(ignis)

include(cmake/CPM.cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Vulkan REQUIRED)

file(GLOB IGNIS_SRC "src/*.cpp")
add_library(ignis STATIC ${IGNIS_SRC})
target_include_directories(ignis PUBLIC include)

add_library(Ignis::Ignis ALIAS ignis)

# TEMP: there should be no dependencies
CPMAddPackage(
  NAME VulkanMemoryAllocator
  GITHUB_REPOSITORY GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
  GIT_TAG v3.2.0
)

target_link_libraries(ignis PUBLIC
  Vulkan::Vulkan
  GPUOpen::VulkanMemoryAllocator
)

# "tests"
option(BUILD_TESTING "Build the tests" ON)

if(NOT BUILD_TESTING)
  return()
endif()

add_executable(ignis_test test/main.cpp)
target_link_libraries(ignis_test PRIVATE Ignis::Ignis)
