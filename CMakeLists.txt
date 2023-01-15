cmake_minimum_required(VERSION 3.10)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/cmake/")
if(POLICY CMP0076)
  cmake_policy(SET CMP0076 NEW)
endif()
# Do not add /W3 to MSVC flags by default. We set this to /W4.
if(POLICY CMP0092)
  cmake_policy(SET CMP0092 NEW)
endif()
if(POLICY CMP0074)
  cmake_policy(SET CMP0074 NEW)
endif()

# Allow vcpkg to use versions feature if it is enabled with a manifest.
# Right now this is done for the Python CIs.
list(APPEND VCPKG_FEATURE_FLAGS "versions")

# Read version into variable
file(READ version.txt PACKAGE_VERSION)
string(STRIP ${PACKAGE_VERSION} PACKAGE_VERSION)
message(STATUS "VowpalWabbit Version: ${PACKAGE_VERSION}")

# version.txt may contain a SemVer of the form 1.2.3-xxx+xxx
# Extract the first 3 numbers into CMake variables
string(REGEX REPLACE "[.+-]" ";" VW_VERSION_LIST ${PACKAGE_VERSION})
list(GET VW_VERSION_LIST 0 VW_VERSION_MAJOR)
list(GET VW_VERSION_LIST 1 VW_VERSION_MINOR)
list(GET VW_VERSION_LIST 2 VW_VERSION_PATCH)

# Set this to on so that tooling can make use of the outputted compile commands (such as clang-tidy)
set(CMAKE_EXPORT_COMPILE_COMMANDS On)

if (NOT CMAKE_BUILD_TYPE AND NOT GENERATOR_IS_MULTI_CONFIG)
  message(STATUS "No build type selected, defaulting to Release")
  set(CMAKE_BUILD_TYPE "Release")
endif()

if(WIN32)
  include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/platforms/win32.cmake)
  message(STATUS "WinSDK Version: ${CMAKE_SYSTEM_VERSION}")
  if (vw_BUILD_NET_FRAMEWORK)
    string(REGEX MATCH "[0-9.]+" VW_NET_FRAMEWORK_VERSION ${CMAKE_DOTNET_TARGET_FRAMEWORK_VERSION})
    string(REPLACE "." "" VW_NET_FRAMEWORK_TFM ${VW_NET_FRAMEWORK_VERSION})
    string(PREPEND VW_NET_FRAMEWORK_TFM "net")
    message(STATUS ".NET Framework Version: ${VW_NET_FRAMEWORK_VERSION} (${VW_NET_FRAMEWORK_TFM})")
  endif()
endif()

option(BUILD_FLATBUFFERS "Build flatbuffers" OFF)
if(BUILD_FLATBUFFERS)
  list(APPEND VCPKG_MANIFEST_FEATURES "flatbuffers")
endif()

option(BUILD_TESTING "Build tests" ON)
if(BUILD_TESTING)
  list(APPEND VCPKG_MANIFEST_FEATURES "tests")
endif()

option(BUILD_BENCHMARKS "Build benchmarks" OFF)
if(BUILD_BENCHMARKS)
  list(APPEND VCPKG_MANIFEST_FEATURES "benchmarks")
endif()

# CMake project() does not support semantic versioning
project(vowpal_wabbit VERSION "${VW_VERSION_MAJOR}.${VW_VERSION_MINOR}.${VW_VERSION_PATCH}" LANGUAGES C CXX)
set(VW_PROJECT_DESCRIPTION "Vowpal Wabbit Machine Learning System")
set(VW_PROJECT_URL "https://vowpalwabbit.org")

option(USE_LATEST_STD "Override using C++11 with the latest standard the compiler offers. Default is C++11. " OFF)
include(DetectCXXStandard)
if(NOT VW_CXX_STANDARD)
  set(VW_CXX_STANDARD 11)
endif()
if(USE_LATEST_STD)
  DetectCXXStandard(VW_CXX_STANDARD)
endif()
message(STATUS "Using C++ standard: " ${VW_CXX_STANDARD})
set(CMAKE_CXX_STANDARD ${VW_CXX_STANDARD})
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_VISIBILITY_INLINES_HIDDEN TRUE)
set(CMAKE_CXX_VISIBILITY_PRESET "hidden")

include(VowpalWabbitUtils)

if(MSVC)
  # Use C++ standard exception handling instead of MSVC default
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
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

option(VW_GCOV "Turn on flags required for gcov" OFF)
option(WARNINGS "Turn on warning flags. ON by default." ON)
option(WARNING_AS_ERROR "Turn on warning as error. OFF by default." OFF)
option(STATIC_LINK_VW "Link VW executable statically. Off by default." OFF)
option(STATIC_LINK_VW_JAVA "Link VW-JNI shared library statically. Off by default." OFF)
option(VW_INSTALL "Add install targets." OFF)
option(BUILD_ONLY_STANDALONE_BENCHMARKS "Build only the benchmarks that can run standalone (and do not use vw internals)" OFF)
option(BUILD_JAVA "Add Java targets." Off)
option(BUILD_PYTHON "Add Python targets." Off)
option(BUILD_EXPERIMENTAL_BINDING "Add targets for experimental bindings" OFF)
option(LTO "Enable Link Time optimization (Requires Release build, only works with clang and linux/mac for now)." Off)
option(RAPIDJSON_SYS_DEP "Override using the submodule for RapidJSON dependency. Instead will use find_package" OFF)
option(FMT_SYS_DEP "Override using the submodule for FMT dependency. Instead will use find_package" OFF)
option(SPDLOG_SYS_DEP "Override using the submodule for spdlog dependency. Instead will use find_package" OFF)
option(VW_BOOST_MATH_SYS_DEP "Override using the submodule for boost math dependency. Instead will use find_package" OFF)
option(VW_ZLIB_SYS_DEP "Override using the submodule for zlib dependency. Instead will use find_package" ON)
option(VW_GTEST_SYS_DEP "Override using fetch package for gtest dependency. Instead will use find_package" OFF)
option(VW_EIGEN_SYS_DEP "Override using the submodule for Eigen3 dependency. Instead will use find_package" OFF)
option(VW_STRING_VIEW_LITE_SYS_DEP "Override using the submodule for string-view-lite dependency. Instead will use find_package" OFF)
option(VW_SSE2NEON_SYS_DEP "Override using the submodule for SSE2Neon dependency. Instead will use find_package" OFF)
option(VW_BUILD_VW_C_WRAPPER "Enable building the c_wrapper project" ON)
option(VW_BUILD_CSV "Build csv parser" OFF)
option(VW_BUILD_LAS_WITH_SIMD "Build large action space with explicit simd (only work with linux for now)" ON)
option(vw_BUILD_NET_CORE "Build .NET Core targets" OFF)
option(vw_BUILD_NET_FRAMEWORK "Build .NET Framework targets" OFF)
option(VW_USE_ASAN "Compile with AddressSanitizer" OFF)
option(VW_USE_UBSAN "Compile with UndefinedBehaviorSanitizer" OFF)

if(VW_USE_ASAN)
  add_compile_definitions(VW_USE_ASAN)
  if(MSVC)
    add_compile_options(/fsanitize=address /GS- /wd5072)
    add_link_options(/InferASanLibs /incremental:no /debug)
    # Workaround for MSVC ASan issue here: https://developercommunity.visualstudio.com/t/VS2022---Address-sanitizer-on-x86-Debug-/10116361
    add_compile_definitions(_DISABLE_STRING_ANNOTATION)
  else()
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer -g3)
    add_link_options(-fsanitize=address -fno-omit-frame-pointer -g3)
  endif()
endif()

if(VW_USE_UBSAN)
  add_compile_definitions(VW_USE_UBSAN)
  if(MSVC)
    message(FATAL_ERROR "UBSan not supported on MSVC")
  else()
    add_compile_options(-fsanitize=undefined -fno-sanitize-recover -fno-omit-frame-pointer -g3)
    add_link_options(-fsanitize=undefined -fno-sanitize-recover -fno-omit-frame-pointer -g3)
  endif()
endif()

if(VW_INSTALL AND NOT VW_ZLIB_SYS_DEP)
  message(WARNING "Installing with a vendored version of zlib is not recommended. Use VW_ZLIB_SYS_DEP to use a system dependency or specify VW_INSTALL=OFF to silence this warning.")
endif()

# The only way to tell it was used is if it was turned off, since the default is true.
if(DEFINED BUILD_TESTS)
  message(WARNING "Value of BUILD_TESTS option ignored. Please use the standard option BUILD_TESTING.")
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

if(VW_GCOV AND (NOT CMAKE_BUILD_TYPE STREQUAL "Debug"))
  message(FATAL_ERROR "VW_GCOV requires Debug build type.")
endif()

if(WIN32 AND (STATIC_LINK_VW OR BUILD_JAVA OR VW_GOV))
  message(FATAL_ERROR "Unsupported option enabled on Windows build")
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
  else()
    set(LINK_THREADS -Wl,--whole-archive -lpthread -Wl,--no-whole-archive)
    set(unix_static_flag -static)
  endif()
endif()

# This provides the variables such as CMAKE_INSTALL_LIBDIR for installation paths.
include(GNUInstallDirs)

add_subdirectory(library)
include(ext_libs/ext_libs.cmake)
add_subdirectory(vowpalwabbit)
add_subdirectory(utl/dump_options)

if (BUILD_FLATBUFFERS)
  add_subdirectory(utl/flatbuffer)
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

# Must be done after the test block so that enable_testing() has been called
if(BUILD_EXPERIMENTAL_BINDING)
  add_subdirectory(bindings)
endif()

if(vw_BUILD_NET_FRAMEWORK OR vw_BUILD_NET_CORE)
  add_subdirectory(cs)
endif()

# Handle installation of targets, version, config and pkgconfig
if(VW_INSTALL)
  # only do this if the file exists. Some packages dont have this file and its only needed in a very specific use case
  if(EXISTS nuget/native/vowpalwabbit.nuspec.in)
    configure_file(nuget/native/vowpalwabbit.nuspec.in nuget/native/vowpalwabbit.nuspec @ONLY)
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
    ${CMAKE_CURRENT_BINARY_DIR}/VowpalWabbitConfig.cmake
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
