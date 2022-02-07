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