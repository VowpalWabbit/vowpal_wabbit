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
  # Don't call add_library - find_package creates an imported target
  target_include_directories(RapidJSON INTERFACE ${RapidJSON_INCLUDE_DIRS} ${RAPIDJSON_INCLUDE_DIRS})
else()
  add_library(RapidJSON INTERFACE)
  target_include_directories(RapidJSON SYSTEM INTERFACE "${CMAKE_CURRENT_LIST_DIR}/rapidjson/include")
endif()

# Boost math only required if LDA is enabled.
if(VW_FEAT_LDA)
  if(VW_BOOST_MATH_SYS_DEP)
    find_package(Boost REQUIRED)
    set(boost_math_target Boost::boost)
  else()
    set(BOOST_MATH_STANDALONE ON CACHE BOOL "Use Boost math vendored dep in standalone mode" FORCE)
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/boost_math EXCLUDE_FROM_ALL)
    set(boost_math_target Boost::math)
  endif()
endif()

if(VW_ZLIB_SYS_DEP)
  find_package(ZLIB REQUIRED)
else()
  file(COPY ${CMAKE_CURRENT_LIST_DIR}/zlib/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/zlib_source/)
  set(CMAKE_POLICY_DEFAULT_CMP0048 NEW)
  set(CMAKE_POLICY_DEFAULT_CMP0042 NEW)
  set(SKIP_INSTALL_ALL ON CACHE BOOL "" FORCE)

  # On Windows, build both static and shared libraries
  # - Static for internal VW components (avoids DLL runtime issues for tests)
  # - Shared for NuGet package distribution
  # On other platforms, build only static to avoid PIC linking issues
  if(WIN32)
    set(ZLIB_BUILD_SHARED ON CACHE BOOL "Build shared zlib on Windows" FORCE)
    set(ZLIB_BUILD_STATIC ON CACHE BOOL "Build static zlib on Windows" FORCE)
  else()
    set(ZLIB_BUILD_SHARED OFF CACHE BOOL "Don't build shared zlib on non-Windows" FORCE)
    set(ZLIB_BUILD_STATIC ON CACHE BOOL "Build static zlib on non-Windows" FORCE)
  endif()

  add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/zlib_source ${CMAKE_CURRENT_BINARY_DIR}/zlib_build ${SHOULD_EXCLUDE_FROM_ALL_TEXT})

  # Always use static library for internal VW components
  target_include_directories(zlibstatic PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/zlib_source>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/zlib_build>
  )
  add_library(ZLIB::ZLIB ALIAS zlibstatic)

  if(VW_INSTALL)
    # Install static library for linking
    install(
      TARGETS zlibstatic
      EXPORT VowpalWabbitConfig
      ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
      LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
      RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

    # On Windows, also install shared library for NuGet package
    if(WIN32)
      install(
        TARGETS zlib
        EXPORT VowpalWabbitConfig
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
    endif()
  endif()
endif()

if (VW_STRING_VIEW_LITE_SYS_DEP)
  find_package(string-view-lite CONFIG REQUIRED)
else()
  add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/string-view-lite)
endif()

if(VW_FEAT_FLATBUFFERS)
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
  # Try modern range syntax first (requires CMake 3.19+, supports Eigen 3.4.0 through 5.x)
  if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.19")
    find_package(Eigen3 3.4.0...5 CONFIG QUIET)
    if(Eigen3_FOUND)
      message(STATUS "Found Eigen3 ${Eigen3_VERSION} using range syntax")
    endif()
  endif()

  if(NOT Eigen3_FOUND)
    # Fallback for older CMake or older Eigen versions
    # Since EXACT is not specified, any version compatible with 3.4.0 is accepted (>= 3.4.0)
    find_package(Eigen3 3.4.0 CONFIG REQUIRED)
    message(STATUS "Found Eigen3 ${Eigen3_VERSION} using legacy syntax")
  endif()
  add_library(eigen ALIAS Eigen3::Eigen)
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

if(VW_FEAT_CB_GRAPH_FEEDBACK)
  add_library(mlpack_ensmallen INTERFACE)
  target_include_directories(mlpack_ensmallen SYSTEM INTERFACE ${CMAKE_CURRENT_LIST_DIR}/armadillo-code/include)

  target_include_directories(mlpack_ensmallen SYSTEM INTERFACE ${CMAKE_CURRENT_LIST_DIR}/ensmallen/include)
endif()
