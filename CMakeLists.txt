cmake_minimum_required(VERSION 3.5)

# Only allow Release and Debug configurations
set(CMAKE_CONFIGURATION_TYPES Debug Release CACHE TYPE INTERNAL FORCE)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
endif()

# VW targets Windows 8.1 SDK
# CMAKE_SYSTEM_VERSION must come before the project is defined in the top level CMakeLists file
# https://stackoverflow.com/questions/45692367/how-to-set-msvc-target-platform-version-with-cmake
if(WIN32)
  set(CMAKE_SYSTEM_VERSION 8.1 CACHE TYPE INTERNAL FORCE)
endif()

project(vowpal_wabbit C CXX)
set(CMAKE_CXX_STANDARD 11)

# Read version into variable
file(READ version.txt PACKAGE_VERSION)
string(STRIP ${PACKAGE_VERSION} PACKAGE_VERSION)
message(STATUS "VowpalWabbit Version: ${PACKAGE_VERSION}")

include(ProcessorCount)
ProcessorCount(NumProcessors)
message(STATUS "Number of processors: ${NumProcessors}")
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/nprocs.txt ${NumProcessors})

option(PROFILE "Turn on flags required for profiling" OFF)
option(VALGRIND_PROFILE "Turn on flags required for profiling with valgrind" OFF)
option(GCOV "Turn on flags required for gcov" OFF)
option(WARNINGS "Turn on warning flags. ON by default." ON)
option(STATIC_LINK_VW "Link VW executable statically. Off by default." OFF)

option(VW_INSTALL "Add install targets." ON)
option(BUILD_TESTS "Build and enable tests." ON)
option(BUILD_JAVA "Add Java targets." Off)
option(BUILD_PYTHON "Add Python targets." Off)
option(BUILD_DOCS "Add documentation targets." Off)
option(LTO "Enable Link Time optimization (Requires Release build, only works with clang and linux/mac for now)." Off)

string(TOUPPER "${CMAKE_BUILD_TYPE}" CONFIG)

if(WIN32 AND (PROFILE OR VALGRIND_PROFILE OR GCOV OR STATIC_LINK_VW OR BUILD_JAVA OR LTO))
  message(FATAL_ERROR "Unsupported option enabled on Windows build")
endif()

# Add -ffast-math for speed, remove for testability.
set(linux_release_config -O3 -fno-strict-aliasing -msse2 -mfpmath=sse)
set(linux_debug_config -g -O0)

if((NOT PROFILE) AND (NOT GCOV))
  set(linux_release_config ${linux_release_config} -fomit-frame-pointer)
endif()

#Use default visiblity on UNIX otherwise a lot of the C++ symbols end up for exported and interpose'able
set(linux_flags -fvisibility=hidden $<$<CONFIG:DEBUG>:${linux_debug_config}> $<$<CONFIG:RELEASE>:${linux_release_config}>)

if(LTO)
	if(NOT "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
		message(FATAL_ERROR "LTO requires Clang")
	endif()
	if("${CMAKE_CXX_COMPILER_VERSION}" VERSION_LESS "8.0.0")
		message(FATAL_ERROR "LTO requires Clang 8.0 (llvm 3.9) or later")
	endif()
	If("${CONFIG}" STREQUAL "DEBUG")
		message(FATAL_ERROR "LTO only works with Release builds")
	endif()
  set(linux_flags ${linux_flags} -flto=thin)
endif()

# for profiling -- note that it needs to be gcc
if(PROFILE)
  set(linux_flags ${linux_flags} -fno-strict-aliasing -pg)
endif()

# for valgrind profiling: run 'valgrind --tool=callgrind PROGRAM' then 'callgrind_annotate --tree=both --inclusive=yes'
if(VALGRIND_PROFILE)
  set(linux_flags ${linux_flags} -g -fomit-frame-pointer -fno-strict-aliasing)
endif()

# gcov configuration
if(GCOV)
  set(linux_flags ${linux_flags} -g -O0 -fprofile-arcs -ftest-coverage -fno-strict-aliasing -pg)
endif()

# Use folders in VS solution
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(explore_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/explore/")

if(STATIC_LINK_VW)
  set(Boost_USE_STATIC_LIBS ON)
  SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
  SET(BUILD_SHARED_LIBS OFF)
else()
  set(Boost_USE_STATIC_LIBS OFF)
  if(WIN32)
    # Windows links everything dynamically
    add_definitions( -DBOOST_ALL_DYN_LINK )
  endif()
endif()

set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

set(LINK_THREADS Threads::Threads)
if(STATIC_LINK_VW)
  if(APPLE)
    set(unix_static_flag "")
    #Guess ZLIB_LIBRARY to be the one provided by homebrew if none was provided
    if(NOT ZLIB_LIBRARY)
      file(GLOB ZLIB_LIBRARY /usr/local/Cellar/zlib/*/lib/libz.a)
    endif()
  else()
    set(LINK_THREADS -Wl,--whole-archive -lpthread -Wl,--no-whole-archive)
    set(unix_static_flag -static)
  endif()
endif()

# Align and foreach are also required, for some reason they can't be specified as components though.
find_package(Boost REQUIRED COMPONENTS program_options system thread unit_test_framework)
find_package(ZLIB REQUIRED)

# Ensure rapidjson submodule is ready
find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
  # Update submodules as needed
  option(GIT_SUBMODULE "Check submodules during build" ON)
  if(GIT_SUBMODULE)
    message(STATUS "Submodule update")
    execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                    RESULT_VARIABLE GIT_SUBMOD_RESULT)
    if(NOT GIT_SUBMOD_RESULT EQUAL "0")
      message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
    endif()
  endif()
endif()

if(NOT EXISTS "${PROJECT_SOURCE_DIR}/rapidjson/CMakeLists.txt")
  message(FATAL_ERROR "The submodules were not downloaded! GIT_SUBMODULE was turned off or failed. Please update submodules and try again.")
endif()

add_library(rapidjson INTERFACE)
target_include_directories(rapidjson INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/rapidjson/include)

add_subdirectory(cluster)
add_subdirectory(library)
add_subdirectory(vowpalwabbit)

if(BUILD_DOCS)
  add_subdirectory(doc)
endif()

if(BUILD_JAVA)
  add_subdirectory(java)
endif()

if(BUILD_PYTHON)
  add_subdirectory(python)
endif()

if(BUILD_TESTS)
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
endif()

# TODO convert cs directory to cmake
# TODO convert c_test directory to cmake

# PkgConfig
if(VW_INSTALL)
  FOREACH(BOOST_LIBRARY ${Boost_LIBRARIES})
    SET (BOOST_LINK_LIBRARIES "${BOOST_LINK_LIBRARIES} ${BOOST_LIBRARY}")
  ENDFOREACH()

  configure_file(libvw.pc.in libvw.pc)
  configure_file(libvw_c_wrapper.pc.in libvw_c_wrapper.pc)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libvw.pc ${CMAKE_CURRENT_BINARY_DIR}/libvw_c_wrapper.pc DESTINATION lib/pkgconfig)
endif()
