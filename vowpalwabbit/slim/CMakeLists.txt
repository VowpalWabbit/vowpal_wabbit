cmake_minimum_required(VERSION 3.5)

project(vwslim_proj CXX)
set(CMAKE_CXX_STANDARD 11)

add_subdirectory(src)

if (BUILD_TESTS)
	enable_testing()
	add_subdirectory(test)
endif ()
