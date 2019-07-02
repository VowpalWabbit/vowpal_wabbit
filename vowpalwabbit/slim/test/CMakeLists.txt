add_executable(vw-slim-test
  ut_main.cc ut_explore.cc ut_opts.cc ut_vw.cc vw_util.cc ut_util.h vw_util.h
)

# Add the include directories from vw target for testing
target_link_libraries(vw-slim-test PRIVATE Boost::unit_test_framework Boost::system)

add_test(
  NAME vw_unit_test
  COMMAND ./vw-unit-test
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
