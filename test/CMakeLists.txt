add_subdirectory(unit_test)
add_subdirectory(tools)

set(FB_FLAG "") # flatbuffers are off by default
if(BUILD_FLATBUFFERS)
  set(FB_FLAG "-F") # include flatbuffer tests in RunTests
endif()

# Don't offer target on Windows
if(NOT WIN32)
  add_test(
    NAME RunTests_pass_1_onethread
    COMMAND
      ${CMAKE_COMMAND} -E env PATH=${CMAKE_CURRENT_BINARY_DIR}/../vowpalwabbit/:$ENV{PATH}
      ./RunTests -d -fe -E 0.001 -O --onethread ${FB_FLAG}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  )

  add_test(
    NAME RunTests_pass_2
    COMMAND
      ${CMAKE_COMMAND} -E env PATH=${CMAKE_CURRENT_BINARY_DIR}/../vowpalwabbit/:$ENV{PATH}
      ./RunTests -d -fe -E 0.001 ${FB_FLAG}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  )

  add_test(
    NAME slow_tests

    COMMAND python3 run_tests.py --clean_dirty --test_spec ${CMAKE_CURRENT_SOURCE_DIR}/slow.vwtest.json
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  )
endif()
