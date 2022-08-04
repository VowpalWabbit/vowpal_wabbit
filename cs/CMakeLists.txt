IF(vw_BUILD_NET_CORE AND vw_BUILD_NET_FRAMEWORK)
  MESSAGE(FATAL_ERROR "Cannot build VW bindings for .NET Core and .NET Framework at the same time.")
ENDIF()

# Explicitly set the `PlatformTarget` for C# projects, since AnyCPU can result in
# System.BadImageFormatException throws, when trying to load C++/CLI assemblies.
IF(CMAKE_GENERATOR_PLATFORM STREQUAL "x64")
  SET(CMAKE_CSharp_FLAGS "/platform:x64")
ELSEIF(CMAKE_GENERATOR_PLATFORM STREQUAL "Win32")
    SET(CMAKE_CSharp_FLAGS "/platform:x86")
ELSE()
  MESSAGE(WARNING "Generator platform is set to '${CMAKE_GENERATOR_PLATFORM}', which is not supported by managed projects. Defaulting to 'AnyCPU'...")
  SET(CMAKE_CSharp_FLAGS "/platform:AnyCPU")
ENDIF()


IF (vw_BUILD_NET_CORE)
  message(STATUS "Building .NET Core Bindings")
  set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
  include(FindDotnet)

  add_subdirectory(common/netstandard)
  add_subdirectory(vw.net.native)
  add_subdirectory(vw.net)
  add_subdirectory(cs/netstandard)
  add_subdirectory(cs_json/netstandard)
  add_subdirectory(cs_parallel/netstandard)
  add_subdirectory(cs_console/netcoreapp)
  add_subdirectory(examples/simulator/netcoreapp)

  if (BUILD_TESTING)
    add_subdirectory(testcommon/netstandard)
    add_subdirectory(unittest/netstandard)
  endif()

ELSEIF (vw_BUILD_NET_FRAMEWORK)
  message(STATUS "Building .NET Framework Bindings")

  # StrongName signing for VW assemblies - stage the keyfile, and set its location
  configure_file(vw_key.snk vw_key.snk COPYONLY)
  set(vw_DOTNET_SIGNING_KEY ${CMAKE_CURRENT_BINARY_DIR}/vw_key.snk)

  add_subdirectory(common)
  add_subdirectory(cli)
  add_subdirectory(cs)
  add_subdirectory(cs_json)
  add_subdirectory(cs_parallel)
  add_subdirectory(cs_console)

  add_subdirectory(examples/simulator)

  if (BUILD_TESTING)
    add_subdirectory(testcommon)
    add_subdirectory(unittest)
    add_subdirectory(cs_unittest_nofriend)
  endif()
ENDIF()
