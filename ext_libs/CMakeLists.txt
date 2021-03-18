# Use position independent code for all external libraries
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(VW_INSTALL)
  set(FMT_INSTALL ON CACHE BOOL "install fmt library" FORCE)
  set(SPDLOG_INSTALL ON CACHE BOOL "install spdlog library" FORCE)
endif()

add_subdirectory(fmt)
set(SPDLOG_FMT_EXTERNAL ON CACHE BOOL "Enable external FMTLIB in spdlog" FORCE)
add_subdirectory(spdlog)

# RapidJson has a bit of logic on which version to use based on the RAPIDJSON_SYS_DEP option
if(RAPIDJSON_SYS_DEP)
  # Since EXACT is not specified, any version compatible with 1.1.0 is accepted (>= 1.1.0)
  find_package(RapidJSON 1.1.0 CONFIG REQUIRED)
  add_library(RapidJSON INTERFACE)
  target_include_directories(RapidJSON INTERFACE ${RAPIDJSON_INCLUDE_DIRS})
else()
  add_library(RapidJSON INTERFACE)
  target_include_directories(RapidJSON INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/rapidjson/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  )

  if(VW_INSTALL)
    install(
      TARGETS RapidJSON
      EXPORT VowpalWabbitConfig)

    install(
      DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/rapidjson/include/rapidjson/
      DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/rapidjson
    )
  endif()
endif()
