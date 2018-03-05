# c_test basic test

add_executable(c_test sample.c)
include_directories(../vowpalwabbit)
vw_c_wrapper_executable_link(c_test)

# Add a tests:
add_test(NAME vw_c_test 
         COMMAND "$<TARGET_FILE:c_test>"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
