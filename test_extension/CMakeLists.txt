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

# Find Boost.Python
find_package(Boost REQUIRED COMPONENTS python3)
message(STATUS "Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")

# Simple extension (pure Python C API)
add_library(simple_module MODULE simple_module.c)
set_target_properties(simple_module PROPERTIES PREFIX "")
target_include_directories(simple_module PRIVATE ${Python_INCLUDE_DIRS})

# Boost.Python extension (to test if Boost.Python is the issue)
add_library(boost_module MODULE boost_module.cc)
set_target_properties(boost_module PROPERTIES PREFIX "")
target_include_directories(boost_module PRIVATE ${Python_INCLUDE_DIRS})
target_link_libraries(boost_module PRIVATE Boost::python3)

if(APPLE)
  # Post-build diagnostics
  add_custom_command(TARGET simple_module POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "=== POST-BUILD Diagnostic: simple_module ==="
    COMMAND otool -hv $<TARGET_FILE:simple_module>
    COMMAND otool -L $<TARGET_FILE:simple_module>
  )
  add_custom_command(TARGET boost_module POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "=== POST-BUILD Diagnostic: boost_module ==="
    COMMAND otool -hv $<TARGET_FILE:boost_module>
    COMMAND otool -L $<TARGET_FILE:boost_module>
  )
endif()
