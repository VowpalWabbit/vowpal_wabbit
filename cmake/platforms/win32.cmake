# This file contains header definitions applicable to Win32
# CMAKE_SYSTEM_VERSION must come before the project is defined in the top level CMakeLists file
# https://stackoverflow.com/questions/45692367/how-to-set-msvc-target-platform-version-with-cmake


if(WIN32)
  # VW targets Windows 10.0.16299.0 SDK
  set(CMAKE_SYSTEM_VERSION "10.0.16299.0" CACHE INTERNAL "Windows SDK version to target.")

  option(vw_BUILD_NET_FRAMEWORK "Build .NET Framework targets" OFF)

  if (vw_BUILD_NET_FRAMEWORK)
    cmake_minimum_required(VERSION 3.14)
    set(CMAKE_DOTNET_TARGET_FRAMEWORK_VERSION  "v4.7.2" CACHE INTERNAL ".NET Framework SDK version to target.")

    # The MSBuild system does not get properly enlightened to C++/CLI projects (for chaining dependencies) when
    # set up through CMake (TODO: How to fix this?). This makes it so native dependencies of the underlying VW
    # native library do not get passed through the project reference properly. The fix is to do the same as on
    # windows and redirect all targets to the same place.
    SET(vw_win32_CMAKE_RUNTIME_OUTPUT_DIRECTORY_backup ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/binaries/")
  endif()
else()
  message(FATAL_ERROR "Loading Win32-specific configuration under a non-Win32 build.")
endif()