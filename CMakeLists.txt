cmake_minimum_required(VERSION 3.14 FATAL_ERROR)
project(Ignis)

include(cmake/CPM.cmake)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_SHARED "Build shared library" OFF)
option(IGNIS_INSTALL "Install the library" ${PROJECT_IS_TOP_LEVEL})

file(GLOB IGNIS_SRC "src/*.cpp")

if (BUILD_SHARED)
  add_library(Ignis SHARED ${IGNIS_SRC})
else()
  add_library(Ignis STATIC ${IGNIS_SRC})
endif()

target_include_directories(Ignis PUBLIC 
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

find_package(Vulkan REQUIRED)

CPMAddPackage(
  NAME Vma
  GITHUB_REPOSITORY GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
  GIT_TAG v3.2.0
)

target_link_libraries(Ignis PUBLIC
  Vulkan::Vulkan
  GPUOpen::VulkanMemoryAllocator
)

if (IGNIS_INSTALL)
  install(DIRECTORY include/ignis DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

  install(FILES ${Vma_SOURCE_DIR}/include/vk_mem_alloc.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  )
endif()
