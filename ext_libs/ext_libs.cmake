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
  target_include_directories(RapidJSON SYSTEM INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/rapidjson/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  )

  if(VW_INSTALL)
    install(
      TARGETS RapidJSON
      EXPORT VowpalWabbitConfig)

    install(
      DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/rapidjson/include/rapidjson/
      DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/rapidjson
    )
  endif()
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

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/string-view-lite)

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

if(VW_BUILD_LARGE_ACTION_SPACE)
  add_library(eigen INTERFACE)
  target_include_directories(eigen SYSTEM INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/eigen>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  )
endif()
