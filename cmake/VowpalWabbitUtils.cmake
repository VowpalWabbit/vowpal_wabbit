include(CMakeParseArguments)
include(GNUInstallDirs)
include(VWFlags)
include(DetectCXXStandard)
if(NOT VW_CXX_STANDARD)
  set(VW_CXX_STANDARD 11)
endif()
if(USE_LATEST_STD)
  DetectCXXStandard(VW_CXX_STANDARD)
endif()

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  include(CTest)
  if(BUILD_TESTING)
    if(${CMAKE_VERSION} VERSION_LESS "3.11.0")
      message(WARNING "BUILD_TESTING requires CMake >= 3.11.0. You can turn if off by setting BUILD_TESTING=OFF")
    endif()
    if(VW_GTEST_SYS_DEP)
      find_package(GTest REQUIRED)
    else()
      cmake_minimum_required(VERSION 3.11)
      include(FetchContent)
      FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/release-1.11.0.zip
      )
      # For Windows: Prevent overriding the parent project's compiler/linker settings
      set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
      set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
      FetchContent_MakeAvailable(googletest)
    endif()
    include(GoogleTest)
  endif()
endif()

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

function(internal_turn_into_build_interface_list LIST_TO_TRANSFORM)
    set(TEMP_LIST "")
    foreach(item IN LISTS ${LIST_TO_TRANSFORM})
        list(APPEND TEMP_LIST "$<BUILD_INTERFACE:${item}>")
    endforeach()
    set(${LIST_TO_TRANSFORM} "${TEMP_LIST}" PARENT_SCOPE)
endfunction()

function(internal_prefix_to_list LIST_TO_TRANSFORM PREFIX)
    set(TEMP_LIST "")
    foreach(item IN LISTS ${LIST_TO_TRANSFORM})
        list(APPEND TEMP_LIST "${PREFIX}${item}")
    endforeach()
    set(${LIST_TO_TRANSFORM} "${TEMP_LIST}" PARENT_SCOPE)
endfunction()


# All libs by default are static only.
# TYPE: STATIC_ONLY, SHARED_ONLY, HEADER_ONLY or STATIC_OR_SHARED
function(vw_add_library)
  cmake_parse_arguments(
    VW_LIB
    "ENABLE_INSTALL"
    "NAME;TYPE;DESCRIPTION;EXCEPTION_DESCRIPTION"
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

  if(NOT DEFINED VW_LIB_DESCRIPTION)
    message(WARNING "No DESCRIPTION specified for ${VW_LIB_NAME}")
  endif()

  if(NOT DEFINED VW_LIB_EXCEPTION_DESCRIPTION)
    message(WARNING "No EXCEPTION_DESCRIPTION specified for ${VW_LIB_NAME}")
  endif()

  # TODO this can be removed when we target a minimum of CMake 3.13
  # https://stackoverflow.com/questions/49996260/how-to-use-target-sources-command-with-interface-library
  foreach(SOURCE IN LISTS VW_LIB_SOURCES)
    if(IS_ABSOLUTE ${SOURCE})
      message(FATAL_ERROR "SOURCES must contain relative paths. Found ${SOURCE}")
    endif()
  endforeach()

  # TODO this can also be removed at 3.13
  internal_prefix_to_list(VW_LIB_SOURCES ${CMAKE_CURRENT_LIST_DIR}/)
  # We must transform after checking the list as IS_ABSOLUTE will not work with generator expression
  internal_turn_into_build_interface_list(VW_LIB_SOURCES)

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

  if(${VW_LIB_TYPE} STREQUAL "HEADER_ONLY")
    # Interface libs can't have sources in CMake 3.10
    add_library(${FULL_LIB_NAME} ${CONCRETE_CMAKE_LIB_TYPE})
  else()
    add_library(${FULL_LIB_NAME} ${CONCRETE_CMAKE_LIB_TYPE} ${VW_LIB_SOURCES})
  endif()

  add_library(VowpalWabbit::${FULL_LIB_NAME} ALIAS ${FULL_LIB_NAME})

  if(VW_OUPUT_LIB_DESCRIPTIONS)
    message(STATUS "{\"name\":\"${VW_LIB_NAME}\", \"target\":\"${FULL_LIB_NAME}\", \"type\":\"${VW_LIB_TYPE}\",\"description\":\"${VW_LIB_DESCRIPTION}\",\"public_deps\":\"${VW_LIB_PUBLIC_DEPS}\",\"private_deps\":\"${VW_LIB_PRIVATE_DEPS}\",\"exceptions\":\"${VW_LIB_EXCEPTION_DESCRIPTION}\"}")
  endif()

  # Append d suffix if we are on Windows and are building a sttic libraru
  if(WIN32)
    if((${VW_LIB_TYPE} STREQUAL "STATIC_ONLY") OR
      ((${VW_LIB_TYPE} STREQUAL "STATIC_OR_SHARED") AND NOT BUILD_SHARED_LIBS))
      set_target_properties(${FULL_LIB_NAME} PROPERTIES DEBUG_POSTFIX d)
    endif()
  endif()

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

  # Older CMake versions seem to not like properties on interface libraries.
  # When the miniumum is upgraded, try removing this.
  if(NOT (${VW_LIB_TYPE} STREQUAL "HEADER_ONLY"))
    set_property(TARGET ${FULL_LIB_NAME} PROPERTY CXX_STANDARD ${VW_CXX_STANDARD})
    set_property(TARGET ${FULL_LIB_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)
    set_property(TARGET ${FULL_LIB_NAME} PROPERTY CMAKE_CXX_EXTENSIONS OFF)
    set_property(TARGET ${FULL_LIB_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)
    target_compile_options(${FULL_LIB_NAME} PRIVATE ${WARNING_OPTIONS} ${WARNING_AS_ERROR_OPTIONS})
    target_compile_definitions(${FULL_LIB_NAME} PRIVATE _FILE_OFFSET_BITS=64)

    if (WIN32)
      target_compile_options(${FULL_LIB_NAME} PRIVATE ${VW_WIN_FLAGS})
    else()
      target_compile_options(${FULL_LIB_NAME} PRIVATE ${VW_LINUX_FLAGS})
    endif()

    if(VW_GCOV)
      target_compile_options(${FULL_LIB_NAME} PRIVATE -fprofile-arcs -ftest-coverage)
      target_link_libraries(${FULL_LIB_NAME} PRIVATE gcov)
    endif()
  endif()

  if(VW_INSTALL AND VW_LIB_ENABLE_INSTALL)
    install(
      TARGETS ${FULL_LIB_NAME}
      EXPORT VowpalWabbitConfig
      ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
      LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
      RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

    install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/include/
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
  endif()
endfunction()

function(vw_add_executable)
  cmake_parse_arguments(VW_EXE
    "ENABLE_INSTALL"
    "NAME;OVERRIDE_BIN_NAME;DESCRIPTION"
    "SOURCES;DEPS"
    ${ARGN}
  )

  if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/include)
    message(FATAL_ERROR "Executable cannot have header dir")
  endif()

  if(NOT VW_EXE_SOURCES)
    message(FATAL_ERROR "No sources specified")
  endif()

  if(NOT DEFINED VW_EXE_DESCRIPTION)
    message(WARNING "No DESCRIPTION specified for ${VW_EXE_NAME}")
  endif()

  vw_get_bin_target(FULL_BIN_NAME ${VW_EXE_NAME})
  add_executable(${FULL_BIN_NAME} ${VW_EXE_SOURCES})

  if(VW_OUPUT_LIB_DESCRIPTIONS)
    message(STATUS "{\"name\":\"${VW_EXE_NAME}\", \"target\":\"${FULL_BIN_NAME}\", \"type\":\"EXECUTABLE\",\"description\":\"${VW_EXE_DESCRIPTION}\",\"public_deps\":\"\",\"private_deps\":\"${VW_EXE_DEPS}\",\"exceptions\":\"N/A\"}")
  endif()

  if(VW_EXE_OVERRIDE_BIN_NAME)
    set_target_properties(${FULL_BIN_NAME} PROPERTIES OUTPUT_NAME ${VW_EXE_OVERRIDE_BIN_NAME})
  endif()

  if(VW_EXE_DEPS)
    target_link_libraries(${FULL_BIN_NAME} PRIVATE ${VW_EXE_DEPS})
  endif()

  set_property(TARGET ${FULL_BIN_NAME} PROPERTY CXX_STANDARD ${VW_CXX_STANDARD})
  set_property(TARGET ${FULL_BIN_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)
  set_property(TARGET ${FULL_BIN_NAME} PROPERTY CMAKE_CXX_EXTENSIONS OFF)
  target_compile_options(${FULL_BIN_NAME} PRIVATE ${WARNING_OPTIONS} ${WARNING_AS_ERROR_OPTIONS})
  target_compile_definitions(${FULL_BIN_NAME} PRIVATE _FILE_OFFSET_BITS=64)
  if (WIN32)
    target_compile_options(${FULL_BIN_NAME} PRIVATE ${VW_WIN_FLAGS})
  else()
    target_compile_options(${FULL_BIN_NAME} PRIVATE ${VW_LINUX_FLAGS})
  endif()

  if(VW_GCOV)
    target_compile_options(${FULL_BIN_NAME} PRIVATE -fprofile-arcs -ftest-coverage)
    target_link_libraries(${FULL_BIN_NAME} PRIVATE gcov)
  endif()

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


set(GTEST_MAIN_FILE_CONTENTS "#include <gtest/gtest.h>\n\
\n\
int main(int argc, char** argv)\n\
{\n\
  ::testing::InitGoogleTest(&argc, argv);\n\
  return RUN_ALL_TESTS();\n\
}\n"
)

function(vw_add_test_executable)
  cmake_parse_arguments(VW_TEST
  ""
  "FOR_LIB"
  "SOURCES;EXTRA_DEPS;COMPILE_DEFS"
  ${ARGN})

  vw_get_lib_target(FULL_FOR_LIB_NAME ${VW_TEST_FOR_LIB})
  if(NOT TARGET ${FULL_FOR_LIB_NAME})
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

    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/${FULL_TEST_NAME}_main.cc "${GTEST_MAIN_FILE_CONTENTS}")

    add_executable(${FULL_TEST_NAME} ${CMAKE_CURRENT_BINARY_DIR}/${FULL_TEST_NAME}_main.cc ${VW_TEST_SOURCES})
    target_link_libraries(${FULL_TEST_NAME} PUBLIC
      ${FULL_FOR_LIB_NAME}
      ${VW_TEST_EXTRA_DEPS}
      GTest::gmock GTest::gtest
    )
    target_compile_definitions(${FULL_TEST_NAME} PRIVATE ${VW_TEST_COMPILE_DEFS})
    target_compile_options(${FULL_TEST_NAME} PRIVATE ${WARNING_OPTIONS} ${WARNING_AS_ERROR_OPTIONS})
    set_property(TARGET ${FULL_TEST_NAME} PROPERTY CXX_STANDARD ${VW_CXX_STANDARD})
    set_property(TARGET ${FULL_TEST_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)
    set_property(TARGET ${FULL_TEST_NAME} PROPERTY CMAKE_CXX_EXTENSIONS OFF)
    gtest_discover_tests(${FULL_TEST_NAME} PROPERTIES LABELS VWTestList DISCOVERY_TIMEOUT 60)
  endif()
endfunction()
