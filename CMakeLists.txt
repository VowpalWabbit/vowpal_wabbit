cmake_minimum_required(VERSION 3.5)

# Only allow Release and Debug configurations
set(CMAKE_CONFIGURATION_TYPES Debug Release CACHE TYPE INTERNAL FORCE)

project(vowpal_wabbit C CXX)
set(CMAKE_CXX_STANDARD 11)

# VW targets Windows 8.1 SDK
if(WIN32)
  set(CMAKE_SYSTEM_VERSION 8.1 CACHE TYPE INTERNAL FORCE)
endif()

# Read version into variable
file(READ version.txt PACKAGE_VERSION)
string(STRIP ${PACKAGE_VERSION} PACKAGE_VERSION)
message("Version: ${PACKAGE_VERSION}")

include(ProcessorCount)
ProcessorCount(NumProcessors)
message("Number of processors: ${NumProcessors}")
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/nprocs.txt ${NumProcessors})

set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build, options are: Debug, Release" FORCE)

option(PROFILE "Turn on flags required for profiling" OFF)
option(VALGRIND_PROFILE "Turn on flags required for profiling with valgrind" OFF)
option(GCOV "Turn on flags required for gcov" OFF)
option(WARNINGS "Turn on warning flags. ON by default." ON)

# Add -ffast-math for speed, remove for testability.
set(linux_release_config -O3 -fomit-frame-pointer -fno-strict-aliasing -msse2 -mfpmath=sse)
set(linux_debug_config -g -O0)
set(linux_flags $<$<CONFIG:DEBUG>:${linux_debug_config}> $<$<CONFIG:RELEASE>:${linux_release_config}>)

# for profiling -- note that it needs to be gcc
if(PROFILE)
  set(linux_flags ${linux_flags} -fno-strict-aliasing -pg)
endif()

# for valgrind profiling: run 'valgrind --tool=callgrind PROGRAM' then 'callgrind_annotate --tree=both --inclusive=yes'
if(VALGRIND_PROFILE)
  set(linux_flags ${linux_flags} -ffast-math -g -fomit-frame-pointer -fno-strict-aliasing)
endif()

# gcov configuration
if(GCOV)
  set(linux_flags ${linux_flags} -g -O0 -fprofile-arcs -ftest-coverage -fno-strict-aliasing -pg)
endif()

set(windows_release_config /GL /O2 /Gy /Oi /Ob2 /Ot /Oy /GT /MD)
set(windows_debug_config /INCREMENTAL /Od /Zi /analyze-)
set(windows_flags $<$<CONFIG:DEBUG>:${windows_debug_config}> $<$<CONFIG:RELEASE>:${windows_release_config}>)

# TODO move to consuming vcpkg of this dependency
add_library(rapidjson INTERFACE)
target_include_directories(rapidjson INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/rapidjson/include)

# Use folders in VS solution
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(explore_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/explore/")

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

set(THREADS_PREFER_PTHREAD_FLAG ON)

# Align and foreach are also required, for some reason they can't be specified as components though.
find_package(Boost REQUIRED COMPONENTS program_options system thread unit_test_framework)
find_package(Threads REQUIRED)
find_package(ZLIB REQUIRED)

add_subdirectory(cluster)
add_subdirectory(library)
add_subdirectory(doc)
add_subdirectory(vowpalwabbit)
add_subdirectory(java)
add_subdirectory(python)

enable_testing()
add_subdirectory(test)

# Don't offer these make dependent targets on Windows
if(NOT WIN32)
  # make bigtests BIG_TEST_ARGS="<args here>"
  add_custom_target(bigtests
    DEPENDS vw
    COMMAND make \${BIG_TEST_ARGS}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/big_tests)
endif()

# TODO convert cs directory to cmake
# TODO convert c_test directory to cmake

# PkgConfig
configure_file(libvw.pc.in libvw.pc)
configure_file(libvw_c_wrapper.pc.in libvw_c_wrapper.pc)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libvw.pc ${CMAKE_CURRENT_BINARY_DIR}/libvw_c_wrapper.pc DESTINATION lib/pkgconfig)
