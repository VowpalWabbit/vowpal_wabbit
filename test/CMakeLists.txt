# TODO convert unit_test and test to proper CMake tests
add_subdirectory(unit_test)

# Don't offer target on Windows
if(NOT WIN32)
  add_test(
    NAME RunTests_pass_1_onethread
    COMMAND ./RunTests -d -fe -E 0.001 -O --onethread ../vowpalwabbit/vw
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  )

  add_test(
    NAME RunTests_pass_2
    COMMAND ./RunTests -d -fe -E 0.001 ../vowpalwabbit/vw
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  )

  add_test(
    NAME save_resume_test
    COMMAND python save_resume_test.py --verbose_on_fail
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  )
endif()

# This target ensures all dependencies are built and also uses verbose mode allowing the test output to be seen.
add_custom_target(check
  COMMAND ${CMAKE_CTEST_COMMAND} --verbose
  DEPENDS ezexample_predict ezexample_predict_threaded ezexample_train library_example test_search search_generate recommend gd_mf_weights vw-unit-test.out
)
