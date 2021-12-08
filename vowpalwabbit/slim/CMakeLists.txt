cmake_minimum_required(VERSION 3.5)
project(vwslim LANGUAGES CXX)

option(BUILD_TESTS "Build and enable tests." ON)

add_subdirectory(../../explore ${CMAKE_CURRENT_BINARY_DIR}/explore)
add_subdirectory(../../ext_libs/fmt ${CMAKE_CURRENT_BINARY_DIR}/fmt)
add_subdirectory(src)

if (BUILD_TESTS)
  enable_testing()
  add_subdirectory(test)
endif ()
