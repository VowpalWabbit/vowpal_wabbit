# Use position independent code for all external libraries
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(FMT_SYS_DEP)
  # fmt is now built against 8.1.1. Its possible earlier versions will also work, but that needs to be tested
  find_package(fmt REQUIRED)
else()
  if(VW_INSTALL)
    set(FMT_INSTALL ON CACHE BOOL "install fmt library" FORCE)
  endif()
  add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/fmt)
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
  add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/spdlog)
endif()

# RapidJson has a bit of logic on which version to use based on the RAPIDJSON_SYS_DEP option
if(RAPIDJSON_SYS_DEP)
  # Since EXACT is not specified, any version compatible with 1.1.0 is accepted (>= 1.1.0)
  find_package(RapidJSON 1.1.0 CONFIG REQUIRED)
  add_library(RapidJSON INTERFACE)
  target_include_directories(RapidJSON INTERFACE ${RapidJSON_INCLUDE_DIRS})
else()
  add_library(RapidJSON INTERFACE)
  target_include_directories(RapidJSON INTERFACE
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

if(VW_ZLIB_SYS_DEP)
  find_package(ZLIB REQUIRED)
else()
  include(${CMAKE_CURRENT_LIST_DIR}/zlib.cmake)
endif()