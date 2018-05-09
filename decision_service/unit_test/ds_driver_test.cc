#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
#   define BOOST_TEST_MODULE Main
#endif

#include "http_server/stdafx.h"
#include "http_server/http_server.h"
#include "ds_driver.h"
#include "ds_config_utility.h"

#include <boost/test/unit_test.hpp>


BOOST_AUTO_TEST_CASE(ranking)
{
	//start a http server that will receive events sent from the eventhub_client
	http_helper http_server;
	http_server.on_initialize(U("http://localhost:8080"));

	//create a simple ds configuration
	auto config = decision_service::utility::config::init_from_json(R"({"eventhub_host":"localhost:8080"})");

	//create the ds driver, and initialize it with the config
	decision_service::driver ds(config);

  auto uuid = "uuid";
  auto context = "context";
  auto invalid_uuid = "";
  auto invalid_context = "";

	decision_service::ranking_response response;

	// request ranking
	BOOST_CHECK_EQUAL(ds.ranking_request(uuid, context, response), decision_service::error_code::success);

	//check expected returned codes
	BOOST_CHECK_EQUAL(ds.ranking_request(invalid_uuid, context, response), decision_service::error_code::invalid_argument);//invalid uuid
	BOOST_CHECK_EQUAL(ds.ranking_request(uuid, invalid_context, response), decision_service::error_code::invalid_argument);//invalid context

	//same tests now but with the api_status
  auto status = new decision_service::api_status();

	//invalid uuid
	ds.ranking_request(uuid, invalid_context, response, status);
	BOOST_CHECK_EQUAL(status->get_error_code(), decision_service::error_code::invalid_argument);

	//invalid context
	ds.ranking_request(invalid_uuid, context, response, status);
	BOOST_CHECK_EQUAL(status->get_error_code(), decision_service::error_code::invalid_argument);
	
	//valid request => status is reset
  decision_service::api_status::try_update(status, -42, "hello");
	ds.ranking_request(uuid, context, response, status);
	BOOST_CHECK_EQUAL(status->get_error_code(), 0);
	BOOST_CHECK_EQUAL(status->get_error_msg(), "");

	//stop the http server
	http_server.on_shutdown();

	delete status;
}

BOOST_AUTO_TEST_CASE(reward)
{
	//start a http server that will receive events sent from the eventhub_client
	http_helper http_server;
	http_server.on_initialize(U("http://localhost:8080"));

	//create a simple ds configuration
	auto config = decision_service::utility::config::init_from_json(R"({"eventhub_host":"localhost:8080"})");

	//create a ds driver, and initialize with configuration
	decision_service::driver ds(config);

	const char*  uuid = "uuid";
	const char*  reward = "reward";
	const char*  invalid_uuid = "";
	const char*  invalid_reward = "";

	// report reward
	BOOST_CHECK_EQUAL(ds.report_outcome(uuid, reward), decision_service::error_code::success);

	// check expected returned codes
	BOOST_CHECK_EQUAL(ds.report_outcome(invalid_uuid, reward), decision_service::error_code::invalid_argument);//invalid uuid
	BOOST_CHECK_EQUAL(ds.report_outcome(uuid, invalid_reward), decision_service::error_code::invalid_argument);//invalid reward

	//check api_status content when errors are returned
	decision_service::api_status* status = new decision_service::api_status();

	//invalid uuid
	ds.report_outcome(invalid_uuid, reward, status);
	BOOST_CHECK_EQUAL(status->get_error_code(), decision_service::error_code::invalid_argument);

	//invalid context
	ds.report_outcome(uuid, invalid_reward, status);
	BOOST_CHECK_EQUAL(status->get_error_code(), decision_service::error_code::invalid_argument);
	
	//valid request => status is not modified
  decision_service::api_status::try_update(status, -42, "hello");
  ds.report_outcome(uuid, reward, status);
	BOOST_CHECK_EQUAL(status->get_error_code(), 0);
	BOOST_CHECK_EQUAL(status->get_error_msg(), "");

	//stop the http server
	http_server.on_shutdown();

	delete status;
}
