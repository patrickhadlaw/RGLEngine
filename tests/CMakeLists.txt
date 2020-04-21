cmake_minimum_required(VERSION 3.12)

enable_testing()

aux_source_directory(. TEST_SRC)

add_custom_target(tests)
foreach(test_source ${TEST_SRC})
	get_filename_component(filename ${test_source} NAME_WE)
	add_executable(${filename} ${test_source})
	target_include_directories(${filename} PUBLIC ${RGLE_INCLUDE_DIRS})
	target_link_libraries(${filename} PUBLIC rgle)
	add_test(${filename} ${filename})
	add_custom_command(
		TARGET ${filename}
  	POST_BUILD
		COMMAND ${filename}
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "Running test: ${filename}" VERBATIM
	)
endforeach()
