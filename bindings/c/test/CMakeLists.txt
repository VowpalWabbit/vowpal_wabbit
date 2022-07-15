if(BUILD_TESTING)
  add_executable(vw_c_api_unit_test
    test_main.cc
    placeholder_test.cc
  )

  target_link_libraries(vw_c_api_unit_test PRIVATE vw_c_api Boost::unit_test_framework)

  add_test(
    NAME vw_c_api_unit_test
    COMMAND ./vw_c_api_unit_test
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  )
endif()
