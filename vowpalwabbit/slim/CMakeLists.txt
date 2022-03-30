cmake_minimum_required(VERSION 3.5)
project(vwslim LANGUAGES CXX)

option(BUILD_TESTS "Build and enable tests." ON)

if(BUILD_TESTS)
  message(WARNING "Use standard option BUILD_TESTING instead of BUILD_TESTS. BUILD_TESTS will be removed in the future.")
  SET(BUILD_TESTING ${BUILD_TESTS} CACHE BOOL "Set value to BUILD_TESTING")
endif()

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    include(CTest)
endif()

add_subdirectory(../../explore ${CMAKE_CURRENT_BINARY_DIR}/explore)
add_subdirectory(src)

if (BUILD_TESTING)
  add_subdirectory(test)
endif ()
