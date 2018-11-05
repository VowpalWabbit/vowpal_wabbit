add_executable(vw-unit-test.out main.cc stable_unique_tests.cc)

# Add the include directories from vw target for testing
target_include_directories(vw-unit-test.out PRIVATE $<TARGET_PROPERTY:vw,INCLUDE_DIRECTORIES>)
target_link_libraries(vw-unit-test.out PRIVATE vw allreduce Boost::unit_test_framework Boost::system)

add_test(
  NAME vw_unit_test
  COMMAND ./vw-unit-test.out
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
