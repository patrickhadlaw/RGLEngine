cmake_minimum_required(VERSION 3.0)

set(RGLE_VERSION_MAJOR 0)
set(RGLE_VERSION_MINOR 2)
set(RGLE_VERSION_REVISION 4)

project(rgle VERSION ${RGLE_VERSION_MAJOR}.${RGLE_VERSION_MINOR}.${RGLE_VERSION_REVISION})

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

configure_file (
  "${PROJECT_SOURCE_DIR}/configuration.h.in"
  "${PROJECT_BINARY_DIR}/configuration.h"
)

if(NOT DEFINED RGLE_INSTALL_PATH)
	set(RGLE_INSTALL_PATH ${PROJECT_BINARY_DIR})
endif(NOT DEFINED RGLE_INSTALL_PATH)
set(RGLE_LOG_LEVEL "DEBUG")

configure_file (
	"${PROJECT_SOURCE_DIR}/rgle.cfg.in"
	"${CMAKE_BINARY_DIR}/rgle.cfg"
)

aux_source_directory(src/rgle RGLE_LIB_SRC)
file(GLOB_RECURSE resources res/*)
file(GLOB_RECURSE shaders shader/*.*)

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

add_subdirectory(lib)

set(RGLE_INCLUDE_DIRS include src ${PROJECT_BINARY_DIR})

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS YES)

add_library(rgle SHARED ${RGLE_LIB_SRC})
target_compile_definitions(rgle PUBLIC RGLE_DLL_BUILD_MODE)

add_custom_command(TARGET rgle
	POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_BINARY_DIR}/bin ${CMAKE_BINARY_DIR}
)

target_link_libraries(rgle ${RGLE_LINK_LIBS})
target_include_directories(rgle PUBLIC ${RGLE_INCLUDE_DIRS})
target_include_directories(rgle SYSTEM PUBLIC ${RGLE_LIB_INCLUDE_DIRS})
set_target_properties(rgle PROPERTIES PUBLIC_HEADER "include/rgle.h")

if (MSVC)
	add_compile_options(/W4 /WX)
else()
	add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

install(TARGETS rgle
  LIBRARY DESTINATION lib
	ARCHIVE DESTINATION bin
	PUBLIC_HEADER DESTINATION include
	COMPONENT LIBRARY
)

add_subdirectory(tests)

add_subdirectory(programs)
