cmake_minimum_required(VERSION 3.15)
project(SimpleExtension)

# On macOS, try to set -undefined dynamic_lookup
if(APPLE)
  set(CMAKE_MODULE_LINKER_FLAGS "-Wl,-undefined,dynamic_lookup" CACHE STRING "Linker flags" FORCE)
  message(STATUS "CMAKE_MODULE_LINKER_FLAGS: ${CMAKE_MODULE_LINKER_FLAGS}")
endif()

find_package(Python 3 COMPONENTS Development.Module REQUIRED)
message(STATUS "Python_INCLUDE_DIRS: ${Python_INCLUDE_DIRS}")
message(STATUS "Python_LIBRARIES: ${Python_LIBRARIES}")

add_library(simple_module MODULE simple_module.c)
set_target_properties(simple_module PROPERTIES PREFIX "")
target_include_directories(simple_module PRIVATE ${Python_INCLUDE_DIRS})

if(APPLE)
  # Post-build diagnostic
  add_custom_command(TARGET simple_module POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "=== POST-BUILD Diagnostic ==="
    COMMAND otool -hv $<TARGET_FILE:simple_module>
    COMMAND otool -L $<TARGET_FILE:simple_module>
  )
endif()
