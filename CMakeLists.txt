cmake_minimum_required(VERSION 3.12)

set(RGLE_VERSION_MAJOR 0)
set(RGLE_VERSION_MINOR 2)
set(RGLE_VERSION_REVISION 4)

project(rgle VERSION ${RGLE_VERSION_MAJOR}.${RGLE_VERSION_MINOR}.${RGLE_VERSION_REVISION})

include(environment.cmake)

configure_file (
  "${PROJECT_SOURCE_DIR}/configuration.h.in"
  "${CMAKE_BINARY_DIR}/configuration.h"
)

if(NOT DEFINED RGLE_INSTALL_PATH)
	set(RGLE_INSTALL_PATH ${PROJECT_BINARY_DIR})
endif(NOT DEFINED RGLE_INSTALL_PATH)
set(RGLE_LOG_LEVEL "DEBUG")

configure_file (
	"${PROJECT_SOURCE_DIR}/rgle.cfg.in"
	"${CMAKE_BINARY_DIR}/bin/rgle.cfg"
)

file(GLOB_RECURSE resources res/*)
file(GLOB_RECURSE shaders shader/*.*)

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

add_subdirectory(lib)

set(RGLE_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/src ${PROJECT_BINARY_DIR})

add_subdirectory(src)

add_subdirectory(tests)

add_subdirectory(programs)
