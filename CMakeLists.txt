cmake_minimum_required(VERSION 3.0)
project(cpp-opengl)

set(CPPOGL_VERSION_MAJOR 0)
set(CPPOGL_VERSION_MINOR 1)
set(CPPOGL_VERSION_REVISION 1)

set(GLEW_LOCATION lib/glew)
set(GLFW_LOCATION lib/glfw)
set(FREETYPE_LOCATION lib/freetype)
set(CIMG_LOCATION lib/cimg)

configure_file (
    "${PROJECT_SOURCE_DIR}/configuration.h.in"
    "${PROJECT_BINARY_DIR}/configuration.h"
)

file(GLOB_RECURSE sources src/*.cpp src/*.h)
file(GLOB_RECURSE resources resources/*)
file(GLOB_RECURSE shader shader/*)
file(GLOB_RECURSE lib lib/*)

add_executable(cpp-opengl cpp-opengl.cpp ${sources})
target_compile_options(cpp-opengl PUBLIC -std=c++11)

add_custom_command(TARGET cpp-opengl POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/bin ${PROJECT_SOURCE_DIR})

file(COPY ${resources} DESTINATION resources)
file(COPY ${shader} DESTINATION shader)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(${GLEW_LOCATION}/build/cmake)
add_subdirectory(${GLFW_LOCATION})
add_subdirectory(${FREETYPE_LOCATION})

find_package(OpenGL REQUIRED)

target_include_directories(cpp-opengl PUBLIC src ${PROJECT_BINARY_DIR} ${GLFW_INCLUDE_DIRS} ${CIMG_LOCATION} ${FREETYPE_LOCATION}/include lib/glm ${OPENGL_INCLUDE_DIRS} ${GLEW_LOCATION}/include)

target_link_libraries(cpp-opengl PUBLIC glew ${OPENGL_LIBRARIES} glfw freetype)