cmake_minimum_required(VERSION 3.14 FATAL_ERROR)
project(ignis)

include(cmake/CPM.cmake)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(IGNIS_BUILD_EXAMPLES "Build ignis examples" ON)

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

target_link_libraries(ignis PUBLIC
  Vulkan::Vulkan
  GPUOpen::VulkanMemoryAllocator
)

if (IGNIS_INSTALL)
	install(TARGETS ignis 
		ARCHIVE DESTINATION lib
		LIBRARY DESTINATION lib
		INCLUDES DESTINATION include
	)
endif()
