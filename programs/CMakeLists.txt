cmake_minimum_required(VERSION 3.12)

aux_source_directory(. PROGRAM_SRC)

add_custom_target(programs)
foreach(program_source ${PROGRAM_SRC})
	get_filename_component(filename ${program_source} NAME_WE)
	add_executable(${filename} ${program_source})
	target_include_directories(${filename} PUBLIC ${RGLE_INCLUDE_DIRS})
	target_link_libraries(${filename} PUBLIC rgle)
endforeach()
