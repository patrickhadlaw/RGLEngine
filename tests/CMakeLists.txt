cmake_minimum_required(VERSION 3.0)

aux_source_directory(. TEST_SRC)

add_custom_target(tests)
foreach(test_source ${TEST_SRC})
	get_filename_component(filename ${test_source} NAME_WE)
	add_executable(${filename} ${test_source})
	target_include_directories(${filename} PUBLIC ${RGLE_INCLUDE_DIRS})
	target_link_libraries(${filename} PUBLIC rgle)
endforeach()
