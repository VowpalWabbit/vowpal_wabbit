project(vwslim CXX)
set(CMAKE_CXX_STANDARD 11)
target_include_directories(vwslim ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/../ ${CMAKE_CURRENT_SOURCE_DIR}/../../explore)
add_definitions(-D EXPLORE_NOEXCEPT -D VW_NOEXCEPT)
add_subdirectory(src)

if (BUILD_TESTS)
	add_subdirectory(test)
endif ()
