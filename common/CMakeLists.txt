add_library(vw_common INTERFACE)
add_library(VowpalWabbit::common ALIAS vw_common)

target_include_directories(vw_common INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include/>
  $<INSTALL_INTERFACE:include/>)

if(BUILD_TESTS)
  add_executable(vw_common_unit_test
    test/test_main.cc
    test/hash_test.cc
    test/rand_test.cc
  )

  target_link_libraries(vw_common_unit_test PRIVATE vw_common Boost::unit_test_framework)

  add_test(
    NAME vw_common_unit_test
    COMMAND ./vw_common_unit_test
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  )
endif()

if(VW_INSTALL)
  install(
    TARGETS vw_common
    EXPORT VowpalWabbitConfig
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

  install(
    DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
endif()
