if(VW_UNIT_TEST_WITH_VALGRIND_INTERNAL)
  find_program(VALGRIND "valgrind" REQUIRED)
endif()

add_executable(vw-unit-test.out
  epsilon_decay_test.cc
  automl_test.cc
  automl_weights_test.cc
  baseline_cb_test.cc
  cats_test.cc
  cats_tree_test.cc
  cats_user_provided_pdf.cc
  cb_explore_adf_test.cc
  cb_large_actions_test.cc
  cb_las_one_pass_svd_test.cc
  cb_las_spanner_test.cc
  ccb_parser_test.cc
  ccb_test.cc
  chain_hashing.cc
  confidence_sequence_test.cc
  continuous_actions_parser_test.cc
  custom_reduction_test.cc
  distributionally_robust_test.cc
  epsilon_test.cc
  example_header_test.cc
  feature_group_test.cc
  loss_functions_test.cc
  main.cc
  minimal_custom_reduction.cc
  multiclass_label_parser_test.cc
  numeric_cast_test.cc
  object_pool_test.cc
  offset_tree_test.cc
  parse_args_test.cc
  parser_test.cc
  pmf_to_pdf_test.cc
  prediction_test.cc
  simulator.cc
  simulator.h
  tag_utils_test.cc
  test_common.cc
  test_common.h
  tutorial_test.cc
)

if (VW_BUILD_LAS_WITH_SIMD AND (UNIX AND NOT APPLE) AND (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64"))
  target_compile_definitions(vw-unit-test.out PUBLIC BUILD_LAS_WITH_SIMD)
endif()

# Add the include directories from vw target for testing
target_link_libraries(vw-unit-test.out PRIVATE vw_core vw_test_common Boost::unit_test_framework)
target_include_directories(vw-unit-test.out PRIVATE $<TARGET_PROPERTY:vw_core,INCLUDE_DIRECTORIES>)

# Automatically set BOOST_TEST_DYN_LINK if the dependency is shared.
get_target_property(boost_test_target_type Boost::unit_test_framework TYPE)
if (boost_test_target_type STREQUAL SHARED_LIBRARY)
  message(STATUS "Boost::unit_test_framework looks to be a shared library. Adding BOOST_TEST_DYN_LINK")
  target_compile_definitions(vw-unit-test.out PRIVATE BOOST_TEST_DYN_LINK)
elseif(boost_test_target_type STREQUAL UNKNOWN_LIBRARY)
  # Try inferring type if vcpkg is used
  if (DEFINED VCPKG_TARGET_TRIPLET)
    if (VCPKG_TARGET_TRIPLET EQUAL "x64-windows" OR VCPKG_TARGET_TRIPLET EQUAL "x86-windows" OR VCPKG_TARGET_TRIPLET EQUAL "arm64-osx-dynamic" OR VCPKG_TARGET_TRIPLET EQUAL "x64-osx-dynamic")
      message(STATUS "Boost::unit_test_framework looks to be a shared library based on vcpkg triplet ${VCPKG_TARGET_TRIPLET}. Adding BOOST_TEST_DYN_LINK")
      target_compile_definitions(vw-unit-test.out PRIVATE BOOST_TEST_DYN_LINK)
    endif()
  # If find_package is used then by default we're looking at a shared dependency unless Boost_USE_STATIC_LIBS was set.
  elseif(NOT Boost_USE_STATIC_LIBS)
    get_target_property(boost_test_path Boost::unit_test_framework LOCATION)
    get_filename_component(boost_test_lib_ext ${boost_test_path} EXT)
    if(NOT ${boost_test_lib_ext} IN_LIST STATIC_LIB_SUFFIXES)
      message(STATUS "Found Boost::unit_test_framework library at: ${boost_test_path}")
      message(STATUS "Looks like a shared library. Adding BOOST_TEST_DYN_LINK")
      target_compile_definitions(vw-unit-test.out PRIVATE BOOST_TEST_DYN_LINK)
    endif()
  endif()
endif()

if(VW_UNIT_TEST_WITH_VALGRIND_INTERNAL)
  add_test(
    NAME vw_unit_test
    COMMAND ${VALGRIND} --error-exitcode=100 --track-origins=yes --leak-check=full $<TARGET_FILE:vw-unit-test.out> "--run_test=!*w_iterations*"
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  )
else()
  if (VW_USE_ASAN OR VW_USE_ASAN)
    add_test(
      NAME vw_unit_test
      COMMAND $<TARGET_FILE:vw-unit-test.out> "--run_test=!*w_iterations*"
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
  else()
    add_test(
        NAME vw_unit_test
        COMMAND $<TARGET_FILE:vw-unit-test.out>
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      )
  endif()
endif()

set_tests_properties(vw_unit_test PROPERTIES LABELS "VWTestList")
