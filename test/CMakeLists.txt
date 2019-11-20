add_subdirectory(unit_test)

# Don't offer target on Windows
if(NOT WIN32)
  add_test(
    NAME RunTests_pass_1_onethread
    COMMAND
      ${CMAKE_COMMAND} -E env PATH=${CMAKE_CURRENT_BINARY_DIR}/../vowpalwabbit/:$ENV{PATH}
      ./RunTests -d -fe -E 0.001 -O --onethread
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  )

  add_test(
    NAME RunTests_pass_2
    COMMAND
      ${CMAKE_COMMAND} -E env PATH=${CMAKE_CURRENT_BINARY_DIR}/../vowpalwabbit/:$ENV{PATH}
      ./RunTests -d -fe -E 0.001
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  )

  add_test(
    NAME save_resume_test
    COMMAND python save_resume_test.py --verbose_on_fail --vw ${CMAKE_CURRENT_BINARY_DIR}/../vowpalwabbit/vw
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  )

  #Two race conditions to worry about
  #
  #Scencario 1: done-then-dispatch
  #1. Parsing thread sets p->done
  #2. learning thread get_example notes end_parsed_examples = used_index
  #3. learning thread early terminates.
  #4. Parsing thread calls dispatch_example.
  #
  #Scenario 2: dispatch-then-done
  #1. parser thread dispatches example
  #2. learner thread consumes examples
  #3. learner thread hangs on p->example_available
  #4. parser thread sets done to true.
  #
  #Fix: Use scenario 2 but have done raise the examples_available flag to unblock the learning thread.
  add_test(
    NAME test_race_condition
    COMMAND  python repeat.py 1024 ${CMAKE_CURRENT_BINARY_DIR}/../vowpalwabbit/vw train-sets/rcv1_small.dat --holdout_after 100
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  )
endif()

# This target ensures all dependencies are built and also uses verbose mode allowing the test output to be seen.
add_custom_target(test_with_output
  COMMAND ${CMAKE_CTEST_COMMAND} --verbose
  DEPENDS spanning_tree vw-unit-test.out vw-bin
)
