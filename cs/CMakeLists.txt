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

add_subdirectory(common)
add_subdirectory(cli)