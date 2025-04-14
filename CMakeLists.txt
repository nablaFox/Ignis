cmake_minimum_required(VERSION 3.14 FATAL_ERROR)
project(ignis)

include(cmake/CPM.cmake)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_SHARED "Build shared library" OFF)
option(IGNIS_INSTALL "Install the library" ${PROJECT_IS_TOP_LEVEL})

file(GLOB IGNIS_SRC "src/*.cpp")

if (BUILD_SHARED)
  add_library(ignis SHARED ${IGNIS_SRC})
else()
  add_library(ignis STATIC ${IGNIS_SRC})
endif()

target_include_directories(ignis PUBLIC 
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

find_package(Vulkan REQUIRED)

CPMAddPackage(
  NAME Vma
  GITHUB_REPOSITORY GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
  GIT_TAG v3.2.0
)

target_link_libraries(ignis 
  PUBLIC Vulkan::Vulkan
  PRIVATE GPUOpen::VulkanMemoryAllocator
)

if (IGNIS_INSTALL)
  install(TARGETS ignis
    EXPORT ignisTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    INCLUDES DESTINATION include
  )

  if (DEFINED ${CMAKE_INSTALL_INCLUDEDIR})
    install(DIRECTORY include/ignis 
      DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
  endif()
endif()
