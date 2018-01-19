cmake_minimum_required (VERSION 3.5)
project (decision_service_test)

set (CMAKE_CXX_STANDARD 14)

enable_testing()

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost ${Boost_VW_VERSION} COMPONENTS unit_test_framework REQUIRED)

# TODO: code coverage

include_directories(${Boost_INCLUDE_DIRS} ${cpprest_INCLUDE_DIRS} ${vw_INCLUDE_DIRS} ${ds_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}) 

add_executable(${PROJECT_NAME} ds_test.cc ds_test_action_probabilities.cc ds_test_explore_epsilon.cc ds_test_explore_softmax.cc)

target_link_libraries(${PROJECT_NAME} PUBLIC vw ds ${Boost_LIBRARIES} ${cpprest_LIBRARY} OpenSSL::SSL OpenSSL::Crypto)

add_test(NAME ${PROJECT_NAME} COMMAND $<TARGET_FILE:${PROJECT_NAME}>)