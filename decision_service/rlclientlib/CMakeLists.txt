cmake_minimum_required (VERSION 3.5)
project (rl)

set(CMAKE_CXX_STANDARD 11)

find_package(cpprestsdk REQUIRED)
find_package(OpenSSL REQUIRED)

include_directories(${cpprest_INCLUDE_DIRS} ${vw_INCLUDE_DIRS} ${explore_INCLUDE_DIRS})
include_directories("${PROJECT_SOURCE_DIR}/../include" ${PROJECT_SOURCE_DIR})

add_library(${PROJECT_NAME} api_status.cc error_callback_fn.cc live_model_impl.cc ranking_event.cc 
	live_model.cc ranking_response.cc ranking_response_impl.cc factory_resolver.cc
	logger/eventhub_client.cc logger/logger.cc
	utility/config_utility.cc utility/config_collection.cc utility/str_util.cc utility/context_helper.cc
	vw_model/safe_vw.cc vw_model/vw_model.cc
	model_mgmt/data_callback_fn.cc model_mgmt/model_downloader.cc model_mgmt/model_mgmt.cc model_mgmt/restapi_data_transport.cc)

target_link_libraries(${PROJECT_NAME} PRIVATE vw ${Boost_LIBRARIES} ${cpprest_LIBRARY} OpenSSL::SSL OpenSSL::Crypto)
