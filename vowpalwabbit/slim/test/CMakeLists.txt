# Copy model files used in unit tests.
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/data/Delay_Margin_AudioNetworkPCR_all_cb_FF8.model
  ${CMAKE_CURRENT_BINARY_DIR}/data/Delay_Margin_AudioNetworkPCR_all_cb_FF8.model
  COPYONLY)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/data/cold_start.model
  ${CMAKE_CURRENT_BINARY_DIR}/data/cold_start.model
  COPYONLY)

# NO_CMAKE_PATH is required because the bundled GTestTargets.cmake doesn't export gmock and uses different casing. So we need to use the one that GTest installed.
find_package(GTest REQUIRED NO_CMAKE_PATH)

add_executable(vw-slim-test
  ut_main.cc
  ut_explore.cc
  ut_opts.cc
  ut_vw.cc
  ut_util.h)

target_link_libraries(vw-slim-test
  vwslim GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)
add_test(NAME vw_unit_test COMMAND vw-slim-test)
