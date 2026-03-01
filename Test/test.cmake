set(TEST_PATH "${CMAKE_CURRENT_LIST_DIR}")

file(GLOB SRC_FILES ${TEST_PATH}/*.cpp)

foreach(src ${SRC_FILES})
	get_filename_component(target ${src} NAME_WE)
	add_executable(${target} ${src})
	target_link_libraries(${target} PRIVATE mlc_m)
endforeach()
