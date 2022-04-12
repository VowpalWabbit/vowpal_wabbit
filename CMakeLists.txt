cmake_minimum_required(VERSION 3.5)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/cmake/")
if(POLICY CMP0076)
  cmake_policy(SET CMP0076 NEW)
endif()
# Do not add /W3 to MSVC flags by default. We set this to /W4.
if(POLICY CMP0092)
  cmake_policy(SET CMP0092 NEW)
endif()

# Allow vcpkg to use versions feature if it is enabled with a manifest.
# Right now this is done for the Python CIs.
list(APPEND VCPKG_FEATURE_FLAGS "versions")

# Read version into variable
file(READ version.txt PACKAGE_VERSION)
string(STRIP ${PACKAGE_VERSION} PACKAGE_VERSION)
message(STATUS "VowpalWabbit Version: ${PACKAGE_VERSION}")

string(REPLACE "." ";" VW_VERSION_LIST ${PACKAGE_VERSION})
list(GET VW_VERSION_LIST 0 VW_VERSION_MAJOR)
list(GET VW_VERSION_LIST 1 VW_VERSION_MINOR)
list(GET VW_VERSION_LIST 2 VW_VERSION_PATCH)

# Set this to on so that tooling can make use of the outputted compile commands (such as clang-tidy)
set(CMAKE_EXPORT_COMPILE_COMMANDS On)

set(DEFAULT_BUILD_TYPE "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${DEFAULT_BUILD_TYPE}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${DEFAULT_BUILD_TYPE}" CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

if(WIN32)
  include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/platforms/win32.cmake)
  message(STATUS "WinSDK Version: ${CMAKE_SYSTEM_VERSION}")
  if (vw_BUILD_NET_FRAMEWORK)
    message(STATUS ".NET Framework Version: ${CMAKE_DOTNET_TARGET_FRAMEWORK_VERSION}")
  endif()
endif()

project(vowpal_wabbit VERSION ${PACKAGE_VERSION} LANGUAGES C CXX)
set(VW_PROJECT_DESCRIPTION "Vowpal Wabbit Machine Learning System")
set(VW_PROJECT_URL "https://vowpalwabbit.org")

option(USE_LATEST_STD "Override using C++11 with the latest standard the compiler offers. Default is C++11. " OFF)
include(DetectCXXStandard)
include(VowpalWabbitUtils)

if(MSVC)
  # Do nothing as the default compile flags for MSVC are okay.
else()
  # We want RelWithDebInfo and Release to be similar. But default RelWithDebInfo
  # is O2 and Release is O3, override that here:
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O3 -g -DNDEBUG")
endif()

# Grab git commitish into variable
IF(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/.git)
  FIND_PACKAGE(Git)
  IF(GIT_FOUND)
    EXECUTE_PROCESS(
      COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
      WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
      OUTPUT_VARIABLE "vw_GIT_COMMIT"
      ERROR_QUIET
      OUTPUT_STRIP_TRAILING_WHITESPACE)
    MESSAGE(STATUS "Git Version: ${vw_GIT_COMMIT}" )
  ELSE(GIT_FOUND)
    SET(vw_GIT_COMMIT "")
  ENDIF(GIT_FOUND)
ELSE(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/.git)
  SET(vw_GIT_COMMIT "")
ENDIF(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/.git)

include(ProcessorCount)
ProcessorCount(NumProcessors)
message(STATUS "Number of processors: ${NumProcessors}")
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/nprocs.txt ${NumProcessors})

option(PROFILE "Turn on flags required for profiling" OFF)
option(VALGRIND_PROFILE "Turn on flags required for profiling with valgrind" OFF)
option(GCOV "Turn on flags required for gcov" OFF)
option(WARNINGS "Turn on warning flags. ON by default." ON)
option(WARNING_AS_ERROR "Turn on warning as error. OFF by default." OFF)
option(STATIC_LINK_VW "Link VW executable statically. Off by default." OFF)
option(STATIC_LINK_VW_JAVA "Link VW-JNI shared library statically. Off by default." OFF)
option(VW_INSTALL "Add install targets." OFF)
option(BUILD_TESTS "Build and enable tests." ON)
option(BUILD_BENCHMARKS "Build benchmarks" OFF)
option(BUILD_ONLY_STANDALONE_BENCHMARKS "Build only the benchmarks that can run standalone (and do not use vw internals)" OFF)
option(BUILD_JAVA "Add Java targets." Off)
option(BUILD_PYTHON "Add Python targets." Off)
option(BUILD_DOCS "Add documentation targets." Off)
option(BUILD_EXPERIMENTAL_BINDING "Add targets for experimental bindings" OFF)
option(LTO "Enable Link Time optimization (Requires Release build, only works with clang and linux/mac for now)." Off)
option(RAPIDJSON_SYS_DEP "Override using the submodule for RapidJSON dependency. Instead will use find_package" OFF)
option(FMT_SYS_DEP "Override using the submodule for FMT dependency. Instead will use find_package" OFF)
option(SPDLOG_SYS_DEP "Override using the submodule for spdlog dependency. Instead will use find_package" OFF)
option(VW_BOOST_MATH_SYS_DEP "Override using the submodule for boost math dependency. Instead will use find_package" OFF)
option(VW_ZLIB_SYS_DEP "Override using the submodule for zlib dependency. Instead will use find_package" ON)
option(BUILD_FLATBUFFERS "Build flatbuffers" OFF)
option(BUILD_PRIVACY_ACTIVATION "Enable privacy activation feature" OFF)

if(VW_INSTALL AND NOT VW_ZLIB_SYS_DEP)
  message(WARNING "Installing with a vendored version of zlib is not recommended. Use VW_ZLIB_SYS_DEP to use a system dependency or specify VW_INSTALL=OFF to silence this warning.")
endif()

# The only way to tell it was used is if it was turned off, since the default is true.
if(NOT BUILD_TESTS)
  message(WARNING "Use standard option BUILD_TESTING instead of BUILD_TESTS. BUILD_TESTS will be removed in the future.")
  SET(BUILD_TESTING ${BUILD_TESTS} CACHE BOOL "Set value to BUILD_TESTING")
endif()

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  include(CTest)
  if(BUILD_TESTING)
    include(FetchContent)
    FetchContent_Declare(
      googletest
      URL https://github.com/google/googletest/archive/refs/tags/release-1.11.0.zip
    )
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
  endif()
endif()

if(VW_INSTALL AND NOT FMT_SYS_DEP)
  message(WARNING "Installing with a vendored version of fmt is not recommended. Use FMT_SYS_DEP to use a system dependency or specify VW_INSTALL=OFF to silence this warning.")
endif()

if(VW_INSTALL AND NOT SPDLOG_SYS_DEP)
  message(WARNING "Installing with a vendored version of spdlog is not recommended. Use SPDLOG_SYS_DEP to use a system dependency or specify VW_INSTALL=OFF to silence this warning.")
endif()

if(VW_INSTALL AND NOT RAPIDJSON_SYS_DEP)
  message(WARNING "Installing with a vendored version of rapidjson is not recommended. Use RAPIDJSON_SYS_DEP to use a system dependency or specify VW_INSTALL=OFF to silence this warning.")
endif()

string(TOUPPER "${CMAKE_BUILD_TYPE}" CONFIG)

# When using Ninja, or CCache GCC and Clang do not output colors as it sees it as a non-terminal output.
# Use this option to force color output in these modes.
option(FORCE_COLORED_OUTPUT "Always produce ANSI-colored output (GNU/Clang only)." OFF)
if(FORCE_COLORED_OUTPUT)
  if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    add_compile_options(-fdiagnostics-color=always)
  elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    add_compile_options(-fcolor-diagnostics)
  endif()
endif()

if(WIN32 AND (PROFILE OR VALGRIND_PROFILE OR GCOV OR STATIC_LINK_VW OR BUILD_JAVA OR LTO))
  message(FATAL_ERROR "Unsupported option enabled on Windows build")
endif()

# This was the previous default but is only valid on x86_64 systems.
set(linux_x86_64_opt_flags "")
if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64")
  set(linux_x86_64_opt_flags -msse2 -mfpmath=sse)
endif()

# Add -ffast-math for speed, remove for testability.
# no-stack-check is added to mitigate stack alignment issue on Catalina where there is a bug with aligning stack-check instructions, and stack-check became default option
set(linux_release_config -O3 -fno-strict-aliasing ${linux_x86_64_opt_flags} -fno-stack-check)
set(linux_debug_config -g -O0 -fno-stack-check)

if((NOT PROFILE) AND (NOT GCOV))
  set(linux_release_config ${linux_release_config} -fomit-frame-pointer)
endif()

#Use default visiblity on UNIX otherwise a lot of the C++ symbols end up for exported and interpose'able
set(linux_flags -fvisibility=hidden $<$<CONFIG:DEBUG>:${linux_debug_config}> $<$<CONFIG:RELEASE>:${linux_release_config}> $<$<CONFIG:RelWithDebInfo>:${linux_release_config}>)

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

if(WIN32)
  set(STATIC_LIB_SUFFIXES ".lib" ".a")
else()
  set(STATIC_LIB_SUFFIXES ".a")
endif()

if(STATIC_LINK_VW_JAVA)
  set(CMAKE_FIND_LIBRARY_SUFFIXES ${STATIC_LIB_SUFFIXES})
elseif(STATIC_LINK_VW)
  set(CMAKE_FIND_LIBRARY_SUFFIXES ${STATIC_LIB_SUFFIXES})
  SET(BUILD_SHARED_LIBS OFF)
endif()

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

# This provides the variables such as CMAKE_INSTALL_LIBDIR for installation paths.
include(GNUInstallDirs)

# Ensure submodules are ready
find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
  # Update submodules as needed
  option(GIT_SUBMODULE "Check submodules during build" ON)
  if(GIT_SUBMODULE)
    message(STATUS "Submodule update")
    # Check if an abritrary file in the submodules already exists. If it does
    # then we assume the submodules are already up to date. This is to avoid
    # taking the git lock aggressively whenever we configure. This can cause
    # issues if VSCode with certain extensions has the workspace open at the
    # same time as you interact with it on the command line.
    if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/ext_libs/spdlog/README.md)
      execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        RESULT_VARIABLE GIT_SUBMOD_RESULT)
      if(NOT GIT_SUBMOD_RESULT EQUAL "0")
        message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
      endif()
    endif()
  endif()
endif()

add_subdirectory(explore)
add_subdirectory(cluster)
add_subdirectory(library)
include(ext_libs/ext_libs.cmake)
add_subdirectory(vowpalwabbit)
add_subdirectory(utl/dump_options)

if (BUILD_FLATBUFFERS)
  add_subdirectory(utl/flatbuffer)
endif()

if(BUILD_DOCS)
  add_subdirectory(doc)
endif()

if(BUILD_JAVA)
  add_subdirectory(java)
endif()

if(BUILD_PYTHON)
  add_subdirectory(python)
endif()

if(BUILD_BENCHMARKS)
  add_subdirectory(test/benchmarks)
endif()

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
  # Boost test is only required if we are building tests
  find_package(Boost REQUIRED COMPONENTS unit_test_framework)
  add_subdirectory(test)

  set(vw_c_api_unit_test_target "")
  if(BUILD_EXPERIMENTAL_BINDING)
    set(vw_c_api_unit_test_target "vw_c_api_unit_test")
  endif()

  set(vw_cs_unittest_targets "")
  if (vw_BUILD_NET_FRAMEWORK)
    set(vw_cs_unittest_targets
      cs_unittest
      cs_unittest_nofriend
    )
  endif()

  # The Visual Studio expression generator
  set(cmake_test_configuration "")
  set(cmake_test_verbosity "--verbose")
  if (${CMAKE_GENERATOR} MATCHES "^Visual Studio")
    set(cmake_test_configuration -C $<CONFIG>)
    # The VS Generator builds fall over in test when STDOUT/ERR contains strings of the form
    # "(<other-stuff, snipped>)?error[^:]*:.*", which our native unit tests output as part of a
    # successful run.
    #
    # This is caused by MSBuild being "helpful":
    # https://stackoverflow.com/questions/48117302/does-the-msbuild-exec-task-search-stdout-for-the-string-error
    #
    # The CMake team has discussed this issue, but are using an MSBuild mechanism that does not support
    # overriding this behaviour. The fix is to disable test output on Visual Studio generator builds.
    set(cmake_test_verbosity "")
  endif()

  # This target ensures all dependencies are built and also uses verbose mode allowing the test output to be seen.
  add_custom_target(test_with_output
    COMMAND ${CMAKE_CTEST_COMMAND} ${cmake_test_configuration} ${cmake_test_verbosity} --output-on-failure
    DEPENDS spanning_tree vw-unit-test.out vw-bin ${vw_c_api_unit_test_target} ${vw_cs_unittest_targets}
  )

  # Don't offer these make dependent targets on Windows
  if(NOT WIN32)
    # make bigtests BIG_TEST_ARGS="<args here>"
    add_custom_target(bigtests
      DEPENDS vw
      COMMAND make \${BIG_TEST_ARGS}
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/big_tests)
  endif()
endif()

# Must be done after the test block so that enable_testing() has been called
if(BUILD_EXPERIMENTAL_BINDING)
  add_subdirectory(bindings)
endif()

if(vw_BUILD_NET_FRAMEWORK)
  add_subdirectory(cs)
endif()


# Handle installation of targets, version, config and pkgconfig
if(VW_INSTALL)
  # only do this if the file exists. Some packages dont have this file and its only needed in a very specific use case
  if(EXISTS nuget/vowpalwabbit.nuspec.in)
    configure_file(nuget/vowpalwabbit.nuspec.in nuget/vowpalwabbit.nuspec @ONLY)
  endif()

  configure_file(libvw_c_wrapper.pc.in libvw_c_wrapper.pc @ONLY)
  install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/libvw_c_wrapper.pc
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

  install(EXPORT VowpalWabbitConfig
    FILE
      VowpalWabbitTargets.cmake
    NAMESPACE
      VowpalWabbit::
    DESTINATION
      ${CMAKE_INSTALL_LIBDIR}/cmake/VowpalWabbit)

  include(CMakePackageConfigHelpers)
  write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/VowpalWabbitConfigVersion.cmake
    VERSION ${PACKAGE_VERSION}
    COMPATIBILITY ExactVersion)

  configure_package_config_file (
    cmake/VowpalWabbitConfig.cmake.in
    ${CMAKE_BINARY_DIR}/VowpalWabbitConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/VowpalWabbit)

  install(
    FILES
      ${CMAKE_CURRENT_BINARY_DIR}/VowpalWabbitConfigVersion.cmake
      ${CMAKE_CURRENT_BINARY_DIR}/VowpalWabbitConfig.cmake
    DESTINATION
      ${CMAKE_INSTALL_LIBDIR}/cmake/VowpalWabbit)

endif()

set(CPACK_PACKAGE_VENDOR "Vowpal Wabbit")
set(CPACK_PACKAGE_NAME "vowpal-wabbit")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Vowpal Wabbit is a machine learning system which pushes the frontier of machine learning with techniques such as online, hashing, allreduce, reductions, learning2search, active, and interactive learning.")
set(CPACK_PACKAGE_VERSION_MAJOR ${VW_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${VW_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${VW_VERSION_PATCH})
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
# TODO Make this an email address or contact page
set(CPACK_PACKAGE_CONTACT "https://github.com/VowpalWabbit/vowpal_wabbit")
# Generates a package dependency list in the deb control file
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS On)

include(CPack)
