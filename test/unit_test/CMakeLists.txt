add_executable(vw-unit-test.out
  automl_test.cc
  automl_weights_test.cc
  baseline_cb_test.cc
  cache_test.cc
  cats_test.cc
  cats_tree_test.cc
  cats_user_provided_pdf.cc
  cb_explore_adf_test.cc
  ccb_parser_test.cc
  ccb_test.cc
  chain_hashing.cc
  continuous_actions_parser_test.cc
  custom_reduction_test.cc
  distributionally_robust_test.cc
  dsjson_parser_test.cc
  epsilon_test.cc
  error_test.cc
  example_header_test.cc
  example_test.cc
  explore_test.cc
  feature_group_test.cc
  guard_test.cc
  initialize_test.cc
  interactions_test.cc
  json_parser_test.cc
  loss_functions_test.cc
  main.cc
  math_test.cc
  minimal_custom_reduction.cc
  multiclass_label_parser_test.cc
  numeric_cast_test.cc
  object_pool_test.cc
  offset_tree_test.cc
  parse_args_test.cc
  parser_test.cc
  pmf_to_pdf_test.cc
  power_test.cc
  prediction_test.cc
  random_test.cc
  scope_exit_test.cc
  simulator.cc
  simulator.h
  slates_parser_test.cc
  slates_test.cc
  stable_unique_test.cc
  tag_utils_test.cc
  test_common.cc
  test_common.h
  tokenize_test.cc
  text_utils_test.cc
  tutorial_test.cc
  v_array_test.cc
  vw_versions_test.cc
  weights_test.cc
)

# Add the include directories from vw target for testing
target_include_directories(vw-unit-test.out PRIVATE $<TARGET_PROPERTY:vw,INCLUDE_DIRECTORIES>)
target_link_libraries(vw-unit-test.out PRIVATE vw Boost::unit_test_framework)

if(NOT DEFINED DO_NOT_BUILD_VW_C_WRAPPER)
  target_sources(vw-unit-test.out PUBLIC vwdll_test.cc)
  target_link_libraries(vw-unit-test.out PRIVATE vw_c_wrapper)

  add_custom_command(TARGET vw-unit-test.out POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      $<TARGET_FILE:vw_c_wrapper>
      $<TARGET_FILE_DIR:vw-unit-test.out>
  )
endif()

# Automatically set BOOST_TEST_DYN_LINK if the dependency is shared.
get_target_property(boost_test_target_type Boost::unit_test_framework TYPE)
if (boost_test_target_type STREQUAL SHARED_LIBRARY)
  message(STATUS "Boost::unit_test_framework looks to be a shared library. Adding BOOST_TEST_DYN_LINK")
  target_compile_definitions(vw-unit-test.out PRIVATE BOOST_TEST_DYN_LINK)
elseif(boost_test_target_type STREQUAL UNKNOWN_LIBRARY)
# If find_package is used then by default we're looking at a shared dependency unless Boost_USE_STATIC_LIBS was set.
  if(NOT Boost_USE_STATIC_LIBS)
    message(STATUS "Boost::unit_test_framework looks to be a shared library. Adding BOOST_TEST_DYN_LINK")
    target_compile_definitions(vw-unit-test.out PRIVATE BOOST_TEST_DYN_LINK)
  endif()
endif()

add_test(
  NAME vw_unit_test
  COMMAND $<TARGET_FILE:vw-unit-test.out>
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
set_tests_properties(vw_unit_test PROPERTIES LABELS "VWTestList")