# install(CODE/SCRIPT) uses generator expressions
if(POLICY CMP0087)
  cmake_policy(SET CMP0087 NEW)
endif()

IF(vw_BUILD_NET_CORE AND vw_BUILD_NET_FRAMEWORK)
  MESSAGE(FATAL_ERROR "Cannot build VW bindings for .NET Core and .NET Framework at the same time.")
ENDIF()

if(vw_BUILD_NET_FRAMEWORK AND NOT WIN32)
  message(FATAL_ERROR ".NET Framework can only be built on Windows")
endif()

# Generate .nuspec files for creating Nuget packages
if(WIN32)
  set(NUGET_RUNTIME_ID "win-x64")
elseif(APPLE)
  # Detect macOS architecture
  if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")
    set(NUGET_RUNTIME_ID "osx-arm64")
  else()
    set(NUGET_RUNTIME_ID "osx-x64")
  endif()
else()
  set(NUGET_RUNTIME_ID "linux-x64")
endif()

if(vw_BUILD_NET_FRAMEWORK)
  configure_file(${CMAKE_CURRENT_LIST_DIR}/../nuget/dotnet/dotnet.nuspec.in nuget/dotnet/dotnet.nuspec @ONLY)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/nuget/dotnet/dotnet.nuspec DESTINATION ./)
endif()

if(vw_BUILD_NET_CORE)
  configure_file(${CMAKE_CURRENT_LIST_DIR}/../nuget/dotnet/dotnetcore.nuspec.in nuget/dotnet/dotnetcore.nuspec @ONLY)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/nuget/dotnet/dotnetcore.nuspec DESTINATION ./)
  configure_file(${CMAKE_CURRENT_LIST_DIR}/../nuget/dotnet/dotnetcore_runtime.nuspec.in nuget/dotnet/dotnetcore_runtime.nuspec @ONLY)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/nuget/dotnet/dotnetcore_runtime.nuspec DESTINATION ./)
  configure_file(${CMAKE_CURRENT_LIST_DIR}/../nuget/dotnet/runtime.json.in nuget/dotnet/runtime.json @ONLY)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/nuget/dotnet/runtime.json DESTINATION ./)
endif()

# Add the appropriate subdirectories depending on if we're build Core or Framework
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
  enable_language(CSharp)

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
