cmake_minimum_required(VERSION 3.0)

set(RGLE_VERSION_MAJOR 0)
set(RGLE_VERSION_MINOR 1)
set(RGLE_VERSION_REVISION 4)

project(rgle VERSION ${RGLE_VERSION_MAJOR}.${RGLE_VERSION_MINOR}.${RGLE_VERSION_REVISION})

set(GLEW_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/lib/glew)
set(GLFW_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/lib/glfw)
set(FREETYPE_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/lib/freetype)
set(STB_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/lib/stb)
set(GLM_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/lib/glm)

configure_file (
    "${PROJECT_SOURCE_DIR}/configuration.h.in"
    "${PROJECT_BINARY_DIR}/configuration.h"
)

aux_source_directory(src RGLE_LIB_SRC)
file(GLOB_RECURSE resources res/*)
file(GLOB shaders shader/*)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_custom_target(copy-shader)
foreach(shader ${shaders})
	file(RELATIVE_PATH shaderfile ${PROJECT_SOURCE_DIR} ${shader})
	configure_file(${shader} ${PROJECT_BINARY_DIR}/${shaderfile} COPYONLY)
endforeach()

add_custom_target(copy-resources)
foreach(resource ${resources})
	file(RELATIVE_PATH resourcefile ${PROJECT_SOURCE_DIR} ${resource})
	configure_file(${resource} ${PROJECT_BINARY_DIR}/${resourcefile} COPYONLY)
endforeach()

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(${GLEW_LOCATION}/build/cmake)
add_subdirectory(${GLFW_LOCATION})
add_subdirectory(${FREETYPE_LOCATION})

find_package(OpenGL REQUIRED)

set(RGLE_INCLUDE_DIRS include src ${PROJECT_BINARY_DIR} ${GLFW_LOCATION}/include ${STB_LOCATION} ${FREETYPE_LOCATION}/include ${GLM_LOCATION} ${GLEW_LOCATION}/include)
set(RGLE_LINK_LIBS glew ${OPENGL_LIBRARIES} glfw freetype)

include_directories(${RGLE_INCLUDE_DIRS})
link_directories(${PROJECT_BINARY_DIR})

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS YES)

add_library(rgle SHARED ${RGLE_LIB_SRC})
target_compile_definitions(rgle PUBLIC RGLE_DLL_BUILD_MODE)

add_custom_command(TARGET rgle
	POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_BINARY_DIR}/bin ${PROJECT_BINARY_DIR}
)

target_link_libraries(rgle ${RGLE_LINK_LIBS})
target_include_directories(rgle PRIVATE ${RGLE_INCLUDE_DIRS})
set_target_properties(rgle PROPERTIES PUBLIC_HEADER "include/rgle.h")

install(TARGETS rgle
    LIBRARY DESTINATION bin
	ARCHIVE DESTINATION bin
	PUBLIC_HEADER DESTINATION include
	COMPONENT LIBRARY
)

aux_source_directory(test TEST_SRC)

add_custom_target(tests)
foreach(test_source ${TEST_SRC})
	get_filename_component(filename ${test_source} NAME_WE)
	add_executable(${filename} ${test_source})
	target_include_directories(${filename} PUBLIC ${RGLE_INCLUDE_DIRS})
	target_link_libraries(${filename} PUBLIC rgle)
endforeach()

aux_source_directory(program PROGRAM_SRC)

add_custom_target(programs)
foreach(program_source ${PROGRAM_SRC})
	get_filename_component(filename ${program_source} NAME_WE)
	add_executable(${filename} ${program_source})
	target_include_directories(${filename} PUBLIC ${RGLE_INCLUDE_DIRS})
	target_link_libraries(${filename} PUBLIC rgle)
endforeach()
