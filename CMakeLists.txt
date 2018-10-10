cmake_minimum_required(VERSION 3.2)

# VW targets Windows 8.1 SDK
if(WIN32)
  set(CMAKE_SYSTEM_VERSION 8.1 CACHE TYPE INTERNAL FORCE)
endif()

project(vowpal_wabbit)
set(PACKAGE_VERSION 8.6.1)

set(CMAKE_CONFIGURATION_TYPES "Debug" "Release" CACHE STRING "" FORCE)

add_library(rapidjson INTERFACE)
target_include_directories(rapidjson INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/rapidjson/include)

set(CMAKE_CXX_STANDARD 11)

# Use folders in VS solution
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(explore_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/explore")

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

set(THREADS_PREFER_PTHREAD_FLAG ON)

# Align and foreach are also required
find_package(Boost REQUIRED COMPONENTS program_options system thread unit_test_framework)
find_package(Threads REQUIRED)
find_package(ZLIB REQUIRED)

add_subdirectory(cluster)
add_subdirectory(library)
add_subdirectory(doc)
add_subdirectory(vowpalwabbit)
add_subdirectory(test)

# Don't offer these make dependent targets on Windows
if(NOT WIN32)

  # make bigtests BIG_TEST_ARGS="<args here>"
  add_custom_target(bigtests
    DEPENDS vw
    COMMAND make \${BIG_TEST_ARGS}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/big_tests)

  add_custom_target(python
    DEPENDS vw
    COMMAND make things
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/python)

  # TODO include java in cmake definition. Must be called manually currently.
  # Could be done by making this a custom command instead so that outputs are specified.
  add_custom_target(java
    DEPENDS vw
    COMMAND make things
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/java)
endif()

# TODO convert cs directory to cmake
# TODO convert c_test directory to cmake
