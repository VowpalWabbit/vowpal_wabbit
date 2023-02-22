# Use position independent code for all external libraries
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# We can only exclude from all if install is turned off.
# If we exclude from all then we lose the ability to install
if (VW_INSTALL)
  set(SHOULD_EXCLUDE_FROM_ALL_TEXT)
else()
  set(SHOULD_EXCLUDE_FROM_ALL_TEXT EXCLUDE_FROM_ALL)
endif()

if(FMT_SYS_DEP)
  # fmt is now built against 8.1.1. Its possible earlier versions will also work, but that needs to be tested
  find_package(fmt REQUIRED)
else()
  if(VW_INSTALL)
    set(FMT_INSTALL ON CACHE BOOL "install fmt library" FORCE)
  endif()
  add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/fmt ${SHOULD_EXCLUDE_FROM_ALL_TEXT})
endif()

if(SPDLOG_SYS_DEP)
  # use header-only mode with system-installed spdlog to ensure a consistent version of fmt is used
  set(spdlog_target spdlog::spdlog_header_only)
  # spdlog is now built against 1.9.2. Its possible earlier versions will also work, but that needs to be tested
  find_package(spdlog CONFIG REQUIRED)
else()
  set(spdlog_target spdlog::spdlog)
  set(SPDLOG_FMT_EXTERNAL ON CACHE BOOL "Enable external FMTLIB in spdlog" FORCE)
  if(VW_INSTALL)
    set(SPDLOG_INSTALL ON CACHE BOOL "install spdlog library" FORCE)
  endif()
  add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/spdlog ${SHOULD_EXCLUDE_FROM_ALL_TEXT})
endif()

# RapidJson has a bit of logic on which version to use based on the RAPIDJSON_SYS_DEP option
if(RAPIDJSON_SYS_DEP)
  # Since EXACT is not specified, any version compatible with 1.1.0 is accepted (>= 1.1.0)
  find_package(RapidJSON 1.1.0 CONFIG REQUIRED)
  add_library(RapidJSON INTERFACE)
  target_include_directories(RapidJSON INTERFACE ${RapidJSON_INCLUDE_DIRS})
else()
  add_library(RapidJSON INTERFACE)
  target_include_directories(RapidJSON SYSTEM INTERFACE "${CMAKE_CURRENT_LIST_DIR}/rapidjson/include")
endif()

if(VW_BOOST_MATH_SYS_DEP)
  find_package(Boost REQUIRED)
  set(boost_math_target Boost::boost)
else()
  set(BOOST_MATH_STANDALONE ON CACHE BOOL "Use Boost math vendored dep in standalone mode" FORCE)
  add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/boost_math EXCLUDE_FROM_ALL)
  set(boost_math_target Boost::math)
endif()

if(VW_ZLIB_SYS_DEP)
  find_package(ZLIB REQUIRED)
else()
  file(COPY ${CMAKE_CURRENT_LIST_DIR}/zlib/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/zlib_source/)
  set(CMAKE_POLICY_DEFAULT_CMP0048 NEW)
  set(CMAKE_POLICY_DEFAULT_CMP0042 NEW)
  set(SKIP_INSTALL_ALL ON CACHE BOOL "" FORCE)
  add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/zlib_source ${CMAKE_CURRENT_BINARY_DIR}/zlib_build ${SHOULD_EXCLUDE_FROM_ALL_TEXT})
  target_include_directories(zlibstatic PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/zlib_source>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/zlib_build>
  )
  add_library(ZLIB::ZLIB ALIAS zlibstatic)

  if(VW_INSTALL)
        install(
          TARGETS zlibstatic
          EXPORT VowpalWabbitConfig
          ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
          LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
          RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
  endif()
endif()

if (VW_STRING_VIEW_LITE_SYS_DEP)
  find_package(string-view-lite CONFIG REQUIRED)
else()
  add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/string-view-lite)
endif()

if(BUILD_FLATBUFFERS)
  find_package(Flatbuffers CONFIG QUIET)
  if(FLATBUFFERS_FOUND)
    get_property(flatc_location TARGET flatbuffers::flatc PROPERTY LOCATION)
  else()
    # Fallback to the old version
    find_package(Flatbuffers MODULE REQUIRED)
    set(flatc_location ${FLATBUFFERS_FLATC_EXECUTABLE})
  endif()
  include(FlatbufferUtils)
endif()

if(VW_EIGEN_SYS_DEP)
  # Since EXACT is not specified, any version compatible with 3.4.0 is accepted (>= 3.4.0)
  find_package(Eigen3 3.4.0 CONFIG REQUIRED)
  add_library(eigen INTERFACE)
  target_include_directories(eigen INTERFACE ${EIGEN3_INCLUDE_DIR})
else()
  if(NOT EXISTS ${CMAKE_CURRENT_LIST_DIR}/eigen/CMakeLists.txt)
  message(SEND_ERROR "The eigen git submodule is not available.\
  Please run `git submodule update --init --recursive` or set VW_EIGEN_SYS_DEP to ON if using a system dependency for eigen")
  endif()
  add_library(eigen INTERFACE)
  target_include_directories(eigen SYSTEM INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/eigen>)
endif()

add_library(sse2neon INTERFACE)
if(VW_SSE2NEON_SYS_DEP)
  find_path(SSE2NEON_INCLUDE_DIRS "sse2neon/sse2neon.h")
  target_include_directories(sse2neon SYSTEM INTERFACE "${SSE2NEON_INCLUDE_DIRS}")
else()
  # This submodule is placed into a nested subdirectory since it exposes its
  # header at the root of the repo rather than its own nested sse2neon/ dir
  target_include_directories(sse2neon SYSTEM INTERFACE "${CMAKE_CURRENT_LIST_DIR}/sse2neon")
endif()
