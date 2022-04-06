include(CMakeParseArguments)

# Given a lib name writes to OUTPUT what the correspinding target name will be
function(vw_get_lib_target OUTPUT LIB_NAME)
  set(${OUTPUT} vw_${LIB_NAME} PARENT_SCOPE)
endfunction()

# Given a lib name writes to OUTPUT what the correspinding test target name will be
function(vw_get_test_target OUTPUT LIB_NAME)
  set(${OUTPUT} vw_${LIB_NAME}_test PARENT_SCOPE)
endfunction()

# Given a bin name writes to OUTPUT what the correspinding target name will be
function(vw_get_bin_target OUTPUT BIN_NAME)
  set(${OUTPUT} vw_${BIN_NAME}_bin PARENT_SCOPE)
endfunction()

# All libs by default are static only.
# TYPE: STATIC_ONLY, SHARED_ONLY, HEADER_ONLY or STATIC_OR_SHARED
function(vw_add_library)
  cmake_parse_arguments(
    VW_LIB
    "ENABLE_INSTALL"
    "NAME;TYPE"
    "SOURCES;PUBLIC_DEPS;PRIVATE_DEPS"
    ${ARGN}
  )

  if((${VW_LIB_TYPE} STREQUAL "HEADER_ONLY") AND VW_LIB_PRIVATE_DEPS)
    message(FATAL_ERROR "Header only lib cannot have private dependencies")
  endif()

  if(NOT EXISTS ${CMAKE_CURRENT_LIST_DIR}/include)
    message(FATAL_ERROR "Library requires at least header dir")
  endif()

  if(NOT EXISTS ${CMAKE_CURRENT_LIST_DIR}/include/vw)
    message(FATAL_ERROR "Root header directory must be vw")
  endif()

  if(NOT EXISTS ${CMAKE_CURRENT_LIST_DIR}/include/vw/${VW_LIB_NAME})
    message(FATAL_ERROR "Second header directory must be ${VW_LIB_NAME}")
  endif()

  if(NOT VW_LIB_SOURCES)
    message(FATAL_ERROR "No sources specified")
  endif()

  # TODO this can be removed when we target a minimum of CMake 3.13
  # https://stackoverflow.com/questions/49996260/how-to-use-target-sources-command-with-interface-library
  foreach(SOURCE IN LISTS VW_LIB_SOURCES)
    if(NOT IS_ABSOLUTE ${SOURCE})
      message(FATAL_ERROR "SOURCES must contain absolute paths. Found ${SOURCE}")
    endif()
  endforeach()

  if(NOT VW_LIB_TYPE)
    message(FATAL_ERROR "TYPE must be defined as one of: STATIC_ONLY, SHARED_ONLY, STATIC_OR_SHARED, HEADER_ONLY")
  endif()

  if(${VW_LIB_TYPE} STREQUAL "SHARED_ONLY")
    set(CONCRETE_CMAKE_LIB_TYPE "SHARED")
  elseif(${VW_LIB_TYPE} STREQUAL "STATIC_ONLY")
    set(CONCRETE_CMAKE_LIB_TYPE "STATIC")
  elseif(${VW_LIB_TYPE} STREQUAL "HEADER_ONLY")
    set(CONCRETE_CMAKE_LIB_TYPE "INTERFACE")
  elseif(${VW_LIB_TYPE} STREQUAL "STATIC_OR_SHARED")
    set(CONCRETE_CMAKE_LIB_TYPE "")
  else()
    message(FATAL_ERROR "TYPE is an invalid value. Must be one of: STATIC_ONLY, SHARED_ONLY, STATIC_OR_SHARED, HEADER_ONLY")
  endif()

  # TODO: if shared is supported, define VWDLL_EXPORTS in build interface only
  # TODO: handle flags in a centralized way maybe?

  vw_get_lib_target(FULL_LIB_NAME ${VW_LIB_NAME})
  add_library(${FULL_LIB_NAME} ${CONCRETE_CMAKE_LIB_TYPE})
  add_library(VowpalWabbit::${VW_LIB_NAME} ALIAS ${FULL_LIB_NAME})

  set(SOURCES_TYPE "PRIVATE")
  if(${VW_LIB_TYPE} STREQUAL "HEADER_ONLY")
    set(SOURCES_TYPE "INTERFACE")
  endif()
  target_sources(${FULL_LIB_NAME} ${SOURCES_TYPE} ${VW_LIB_SOURCES})

  set(PUBLIC_LINK_TYPE "PUBLIC")
  if(${VW_LIB_TYPE} STREQUAL "HEADER_ONLY")
    set(PUBLIC_LINK_TYPE "INTERFACE")
  endif()
  if(VW_LIB_PUBLIC_DEPS)
    target_link_libraries(${FULL_LIB_NAME} ${PUBLIC_LINK_TYPE}
                          ${VW_LIB_PUBLIC_DEPS})
  endif()
  if(VW_LIB_PRIVATE_DEPS)
    target_link_libraries(${FULL_LIB_NAME} PRIVATE ${VW_LIB_PRIVATE_DEPS})
  endif()

  target_include_directories(
    ${FULL_LIB_NAME} ${PUBLIC_LINK_TYPE}
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

  set_property(TARGET ${FULL_LIB_NAME} PROPERTY CXX_STANDARD 11)
  set_property(TARGET ${FULL_LIB_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)
  set_property(TARGET ${FULL_LIB_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)

  if(VW_INSTALL AND VW_LIB_ENABLE_INSTALL)
    install(
      TARGETS ${FULL_LIB_NAME}
      EXPORT VowpalWabbitConfig
      ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
      LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
      RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

    install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/include
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/)
  endif()
endfunction()

function(vw_add_executable)
  cmake_parse_arguments(VW_EXE
    "ENABLE_INSTALL"
    "NAME;OVERRIDE_BIN_NAME"
    "SOURCES;DEPS"
    ${ARGN}
  )

  if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/include)
    message(FATAL_ERROR "Executable cannot have header dir")
  endif()

  if(NOT VW_EXE_SOURCES)
    message(FATAL_ERROR "No sources specified")
  endif()

  vw_get_bin_target(FULL_BIN_NAME ${VW_EXE_NAME})
  add_executable(${FULL_BIN_NAME})
  target_sources(${FULL_BIN_NAME} PRIVATE ${VW_EXE_SOURCES})

  if(VW_EXE_OVERRIDE_BIN_NAME)
    set_target_properties(${FULL_BIN_NAME} PROPERTIES OUTPUT_NAME ${VW_EXE_OVERRIDE_BIN_NAME})
  endif()

  if(VW_EXE_DEPS)
    target_link_libraries(${FULL_BIN_NAME} PRIVATE ${VW_EXE_DEPS})
  endif()

  set_property(TARGET ${FULL_BIN_NAME} PROPERTY CXX_STANDARD 11)
  set_property(TARGET ${FULL_BIN_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)

  if(VW_INSTALL AND VW_EXE_ENABLE_INSTALL)
    install(
      TARGETS ${FULL_BIN_NAME}
      EXPORT VowpalWabbitConfig
      ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
      LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
      RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
  endif()
endfunction()

function(vw_add_test_executable)
  cmake_parse_arguments(VW_TEST
  ""
  "FOR_LIB"
  "SOURCES;EXTRA_DEPS"
  ${ARGN})

  if(NOT TARGET VowpalWabbit::${VW_TEST_FOR_LIB})
    message(FATAL_ERROR "Target ${VW_TEST_FOR_LIB} does not exist")
  endif()

  if(NOT VW_TEST_SOURCES)
    message(FATAL_ERROR "No sources specified")
  endif()

  # Only add tests if this is the main cmake project.
  # This just makes sure that we don't add ourselves to the tests of a repo
  # using this one as a subdirectory.
  if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)

    vw_get_test_target(FULL_TEST_NAME ${VW_TEST_FOR_LIB})
    add_executable(${FULL_TEST_NAME})
    target_sources(${FULL_TEST_NAME} PRIVATE ${VW_TEST_SOURCES})
    target_link_libraries(${FULL_TEST_NAME} PUBLIC
      VowpalWabbit::${VW_TEST_FOR_LIB}
      ${VW_TEST_EXTRA_DEPS}
      gtest_main
      gmock
    )
    add_test(NAME ${FULL_TEST_NAME} COMMAND ${FULL_TEST_NAME})
  endif()
endfunction()
