cmake_minimum_required (VERSION 3.5)
project (rl)

set(CMAKE_CXX_STANDARD 11)

find_package(cpprestsdk REQUIRED)
find_package(OpenSSL REQUIRED)

include_directories(${cpprest_INCLUDE_DIRS} ${vw_INCLUDE_DIRS} ${explore_INCLUDE_DIRS})
include_directories("${PROJECT_SOURCE_DIR}/../include")

add_library(${PROJECT_NAME} api_status.cc error_callback_fn.cc live_model_impl.cc ranking_event.cc restapi_data_transport.cc 
	config_collection.cc live_model.cc ranking_response.cc ranking_response_impl.cc str_util.cc
	logger/eventhub_client.cc logger/logger.cc
	utility/config_utility.cc
	model_mgmt/model_mgmt.cc model_mgmt/safe_vw.cc)

target_link_libraries(${PROJECT_NAME} PRIVATE vw ${Boost_LIBRARIES} ${cpprest_LIBRARY} OpenSSL::SSL OpenSSL::Crypto)
