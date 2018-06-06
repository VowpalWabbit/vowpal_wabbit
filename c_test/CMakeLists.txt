project (c_test)

add_executable(${PROJECT_NAME} sample.c)
include_directories(${vw_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} vwdll)

# Add a tests:
add_test(NAME vw_c_test 
         COMMAND "$<TARGET_FILE:c_test>"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
