if(VW_INSTALL)
  set(FMT_INSTALL ON CACHE BOOL "install fmt library" FORCE)
  set(SPDLOG_INSTALL ON CACHE BOOL "install spdlog library" FORCE)
endif()

add_subdirectory(fmt)
set(SPDLOG_FMT_EXTERNAL_HO ON CACHE BOOL "Enable external FMTLIB in spdlog" FORCE)
add_subdirectory(spdlog)

# RapidJson has a bit of logic on which version to use based on the RAPIDJSON_SYS_DEP option
# and updating/initializing the submodule if necessary
if(RAPIDJSON_SYS_DEP)
  # Since EXACT is not specified, any version compatible with 1.1.0 is accepted (>= 1.1.0)
  find_package(RapidJSON 1.1.0 CONFIG REQUIRED)
  add_library(RapidJSON INTERFACE)
  target_include_directories(RapidJSON INTERFACE ${RAPIDJSON_INCLUDE_DIRS})
else()
  # Ensure rapidjson submodule is ready
  find_package(Git QUIET)
  if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
    # Update submodules as needed
    option(GIT_SUBMODULE "Check submodules during build" ON)
    if(GIT_SUBMODULE)
      message(STATUS "Submodule update")
      execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                      RESULT_VARIABLE GIT_SUBMOD_RESULT)
      if(NOT GIT_SUBMOD_RESULT EQUAL "0")
        message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
      endif()
    endif()
  endif()

  if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/rapidjson/CMakeLists.txt")
    message(FATAL_ERROR "The submodules were not downloaded! GIT_SUBMODULE was turned off or failed. Please update submodules and try again.")
  endif()
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
