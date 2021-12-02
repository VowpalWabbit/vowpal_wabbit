add_subdirectory(unit_test)
add_subdirectory(tools)

set(FB_FLAG "") # flatbuffers are off by default
if(BUILD_FLATBUFFERS)
  set(FB_FLAG "--include_flatbuffers") # include flatbuffer tests in RunTests
endif()

# Don't offer target on Windows
if(NOT WIN32)
  add_test(
    NAME RunTests_pass_1_onethread
    COMMAND python3 run_tests.py --fuzzy_compare --exit_first_fail --epsilon 0.001 --ignore_dirty --extra_options=--onethread ${FB_FLAG}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  )

  add_test(
    NAME RunTests_pass_2
    COMMAND python3 run_tests.py --fuzzy_compare --exit_first_fail --epsilon 0.001 --ignore_dirty ${FB_FLAG}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  )

  add_test(
    NAME slow_tests

    COMMAND python3 run_tests.py --ignore_dirty --test_spec ${CMAKE_CURRENT_SOURCE_DIR}/slow.vwtest.json
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  )

  if(BUILD_PRIVACY_ACTIVATION)
    add_test(
      NAME privacy_activation_tests

      COMMAND python3 run_tests.py --ignore_dirty --test_spec ${CMAKE_CURRENT_SOURCE_DIR}/privacy_activation.vwtest.json
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
  endif()
endif()
