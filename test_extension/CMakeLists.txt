# Only set project/cmake_minimum_required when building standalone
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  cmake_minimum_required(VERSION 3.15)
  project(SimpleExtension)
endif()

# Use C++14 for Boost.Python compatibility (Boost uses auto return types)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# On macOS, use -undefined dynamic_lookup for MODULE libraries
# Set it globally for this subdirectory
if(APPLE)
  # Don't use CACHE - set it directly for this scope
  set(CMAKE_MODULE_LINKER_FLAGS "-Wl,-undefined,dynamic_lookup")
  message(STATUS "test_extension: CMAKE_MODULE_LINKER_FLAGS: ${CMAKE_MODULE_LINKER_FLAGS}")
endif()

message(STATUS "test_extension: Configuring test extension subdirectory")

# Python should already be found by parent project, but find it again if needed
if(NOT Python_FOUND)
  find_package(Python 3 COMPONENTS Development.Module REQUIRED)
endif()
message(STATUS "test_extension: Python_FOUND=${Python_FOUND}")
message(STATUS "test_extension: Python_INCLUDE_DIRS=${Python_INCLUDE_DIRS}")
message(STATUS "test_extension: Python_LIBRARIES=${Python_LIBRARIES}")
message(STATUS "test_extension: Python_EXECUTABLE=${Python_EXECUTABLE}")

# Simple extension (pure Python C API)
add_library(simple_module MODULE simple_module.c)
set_target_properties(simple_module PROPERTIES PREFIX "")
target_include_directories(simple_module PRIVATE ${Python_INCLUDE_DIRS})
# Use target_link_options for modern CMake
if(APPLE)
  target_link_options(simple_module PRIVATE "LINKER:-undefined,dynamic_lookup")
endif()

# Boost.Python extension (optional - to test if Boost.Python is the issue)
# Use same logic as VW: BOOST_PY_VERSION_SUFFIX defaults to "3" for Python 3
if(NOT DEFINED BOOST_PY_VERSION_SUFFIX)
  set(BOOST_PY_VERSION_SUFFIX "3")
endif()
message(STATUS "Looking for Boost.Python with suffix: ${BOOST_PY_VERSION_SUFFIX}")

find_package(Boost COMPONENTS python${BOOST_PY_VERSION_SUFFIX})
if(Boost_FOUND)
  message(STATUS "Boost.Python${BOOST_PY_VERSION_SUFFIX} found - building boost_module")
  message(STATUS "Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")
  add_library(boost_module MODULE boost_module.cc)
  set_target_properties(boost_module PROPERTIES PREFIX "")
  target_include_directories(boost_module PRIVATE ${Python_INCLUDE_DIRS})
  target_link_libraries(boost_module PRIVATE Boost::python${BOOST_PY_VERSION_SUFFIX})
  # Use target_link_options for modern CMake
  if(APPLE)
    target_link_options(boost_module PRIVATE "LINKER:-undefined,dynamic_lookup")
  endif()

  # VW test module - links to vw_core like pylibvw does
  # This tests if linking to VW libraries causes the symbol resolution issue
  message(STATUS "test_extension: Checking for vw_core target...")
  if(TARGET vw_core)
    message(STATUS "test_extension: ✓ vw_core TARGET FOUND - building vw_test_module")
    add_library(vw_test_module MODULE vw_test_module.cc)
    set_target_properties(vw_test_module PROPERTIES PREFIX "")
    target_include_directories(vw_test_module PRIVATE ${Python_INCLUDE_DIRS})
    target_link_libraries(vw_test_module PRIVATE Boost::python${BOOST_PY_VERSION_SUFFIX} vw_core)
    # Use target_link_options for modern CMake
    if(APPLE)
      target_link_options(vw_test_module PRIVATE "LINKER:-undefined,dynamic_lookup")
    endif()

    # Make pylibvw depend on vw_test_module so it gets built during wheel build
    if(TARGET pylibvw)
      add_dependencies(pylibvw vw_test_module)
      message(STATUS "test_extension: Added vw_test_module as dependency of pylibvw")
    endif()

    # TEST IMMEDIATELY AFTER BUILD - This is the critical test!
    # Use a CMake script to avoid shell escaping issues
    # Use $<TARGET_FILE_DIR:vw_test_module> to get the actual output directory
    add_custom_command(TARGET vw_test_module POST_BUILD
      COMMAND ${CMAKE_COMMAND}
        -DTEST_DIR=$<TARGET_FILE_DIR:vw_test_module>
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_vw_module.cmake
      COMMENT "Testing vw_test_module import (links to vw_core)"
      VERBATIM
    )
  else()
    message(STATUS "vw_core not found - skipping vw_test_module (only built as part of VW build)")
  endif()
else()
  message(STATUS "Boost.Python${BOOST_PY_VERSION_SUFFIX} not found - skipping boost_module and vw_test_module")
endif()

if(APPLE)
  # Post-build diagnostics
  add_custom_command(TARGET simple_module POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "=== POST-BUILD Diagnostic: simple_module ==="
    COMMAND otool -hv $<TARGET_FILE:simple_module>
    COMMAND otool -L $<TARGET_FILE:simple_module>
  )
  if(Boost_FOUND)
    add_custom_command(TARGET boost_module POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E echo "=== POST-BUILD Diagnostic: boost_module ==="
      COMMAND otool -hv $<TARGET_FILE:boost_module>
      COMMAND otool -L $<TARGET_FILE:boost_module>
    )
    if(TARGET vw_test_module)
      add_custom_command(TARGET vw_test_module POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "=== POST-BUILD Diagnostic: vw_test_module ==="
        COMMAND otool -hv $<TARGET_FILE:vw_test_module>
        COMMAND otool -L $<TARGET_FILE:vw_test_module>
      )
    endif()
  endif()
endif()
