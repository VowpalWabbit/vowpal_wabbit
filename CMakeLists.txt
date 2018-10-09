cmake_minimum_required(VERSION 3.2)

set(PACKAGE_VERSION 8.6.1)
project(vowpal_wabbit)

add_library(rapidjson INTERFACE)
target_include_directories(rapidjson INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/rapidjson/include)

find_package(ZLIB REQUIRED)
add_library(zlib UNKNOWN IMPORTED)
set_target_properties(zlib PROPERTIES
  IMPORTED_LOCATION "${ZLIB_LIBRARIES}"
  INTERFACE_INCLUDE_DIRECTORIES "${ZLIB_INCLUDE_DIRS}"
)

set(CMAKE_CXX_STANDARD 11)

set(explore_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/explore")

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Boost REQUIRED COMPONENTS program_options system thread unit_test_framework)
find_package(Threads REQUIRED)
find_package(ZLIB REQUIRED)

add_subdirectory(cluster)
add_subdirectory(library)
add_subdirectory(doc)
add_subdirectory(vowpalwabbit)
add_subdirectory(test)

# make bigtests BIG_TEST_ARGS="<args here>"
add_custom_target(bigtests DEPENDS vw
  COMMAND make \${BIG_TEST_ARGS}
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/big_tests)

add_custom_target(python COMMAND make things WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/python)
add_custom_target(java COMMAND make things WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/java)

# TODO cs
# TODO c_test

# TODO workout install
