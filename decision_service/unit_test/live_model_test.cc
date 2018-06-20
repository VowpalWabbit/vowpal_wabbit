#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
#   define BOOST_TEST_MODULE Main
#endif

#include <thread>
#include <boost/test/unit_test.hpp>

#include "http_server/stdafx.h"
#include "http_server/http_server.h"
#include "live_model.h"
#include "config_utility.h"
#include "api_status.h"
#include "ranking_response.h"
#include "err_constants.h"
#include "constants.h"

namespace r = reinforcement_learning;
namespace err = reinforcement_learning::error_code;
namespace cfg = reinforcement_learning::utility::config;

const auto JSON_CFG = R"(
  {
    "ApplicationID": "rnc-123456-a",
    "EventHubInteractionConnectionString": "Endpoint=sb://localhost:8080/;SharedAccessKeyName=RMSAKey;SharedAccessKey=<ASharedAccessKey>=;EntityPath=interaction",
    "EventHubObservationConnectionString": "Endpoint=sb://localhost:8080/;SharedAccessKeyName=RMSAKey;SharedAccessKey=<ASharedAccessKey>=;EntityPath=observation",
    "IsExplorationEnabled": true,
    "ModelBlobUri": "http://localhost:8080",
    "InitialExplorationEpsilon": 1.0
  }
  )";
const auto JSON_CONTEXT = R"({"_multi":[{},{}]})";

BOOST_AUTO_TEST_CASE(live_model_ranking_request)
{
	//start a http server that will receive events sent from the eventhub_client
	http_helper http_server;
	http_server.on_initialize(U("http://localhost:8080"));
  r::api_status status;

	//create a simple ds configuration
	auto config = cfg::create_from_json(JSON_CFG);
  config.set(r::name::EH_TEST, "true");

	//create the ds live_model, and initialize it with the config
	r::live_model ds(config);
  BOOST_CHECK_EQUAL(ds.init(&status), err::success);

  const auto uuid = "uuid";
  const auto invalid_uuid = "";
  const auto invalid_context = "";

  r::ranking_response response;

	// request ranking
	BOOST_CHECK_EQUAL(ds.choose_rank(uuid, JSON_CONTEXT, response), err::success);

	//check expected returned codes
	BOOST_CHECK_EQUAL(ds.choose_rank(invalid_uuid, JSON_CONTEXT, response), err::invalid_argument);//invalid uuid
	BOOST_CHECK_EQUAL(ds.choose_rank(uuid, invalid_context, response), err::invalid_argument);//invalid context

	//invalid uuid
	ds.choose_rank(uuid, invalid_context, response, &status);
	BOOST_CHECK_EQUAL(status.get_error_code(), err::invalid_argument);
  
	//invalid context
	ds.choose_rank(invalid_uuid, JSON_CONTEXT, response, &status);
	BOOST_CHECK_EQUAL(status.get_error_code(), err::invalid_argument);
	
	//valid request => status is reset
  r::api_status::try_update(&status, -42, "hello");
	ds.choose_rank(uuid, JSON_CONTEXT, response, &status);
	BOOST_CHECK_EQUAL(status.get_error_code(), 0);
	BOOST_CHECK_EQUAL(status.get_error_msg(), "");

	//stop the http server
	http_server.on_shutdown();
}

BOOST_AUTO_TEST_CASE(live_model_reward)
{
	//start a http server that will receive events sent from the eventhub_client
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  http_helper http_server;
	http_server.on_initialize(U("http://localhost:8080"));
  std::this_thread::sleep_for(std::chrono::milliseconds(500));

	//create a simple ds configuration
	auto config = cfg::create_from_json(JSON_CFG);
  config.set(r::name::EH_TEST, "true");  

	//create a ds live_model, and initialize with configuration
  r::live_model ds(config);

  //check api_status content when errors are returned
  r::api_status status;

  BOOST_CHECK_EQUAL(ds.init(&status), err::success);
  BOOST_CHECK_EQUAL(status.get_error_code(), err::success);
  BOOST_CHECK_EQUAL(status.get_error_msg(), "");

  const auto uuid = "uuid";
	const auto  reward = "reward";
	const auto  invalid_uuid = "";
	const auto  invalid_reward = "";

	// report reward
  const auto scode = ds.report_outcome(uuid, reward, &status);
  BOOST_CHECK_EQUAL(scode, err::success);
  BOOST_CHECK_EQUAL(status.get_error_msg(), "");

	// check expected returned codes
	BOOST_CHECK_EQUAL(ds.report_outcome(invalid_uuid, reward), err::invalid_argument);//invalid uuid
	BOOST_CHECK_EQUAL(ds.report_outcome(uuid, invalid_reward), err::invalid_argument);//invalid reward

	//invalid uuid
	ds.report_outcome(invalid_uuid, reward, &status);
	BOOST_CHECK_EQUAL(status.get_error_code(), reinforcement_learning::error_code::invalid_argument);

	//invalid context
	ds.report_outcome(uuid, invalid_reward, &status);
	BOOST_CHECK_EQUAL(status.get_error_code(), reinforcement_learning::error_code::invalid_argument);
	
	//valid request => status is not modified
  r::api_status::try_update(&status, -42, "hello");
  ds.report_outcome(uuid, reward, &status);
	BOOST_CHECK_EQUAL(status.get_error_code(), err::success);
	BOOST_CHECK_EQUAL(status.get_error_msg(), "");

	//stop the http server
	http_server.on_shutdown();
}

namespace r = reinforcement_learning;

class wrong_class {};

class algo_server {
  public:
    algo_server() : _err_count{0} {}
    void ml_error_handler(void) { shutdown(); }
    int _err_count;
  private:
    void shutdown() { ++_err_count; }
};

void algo_error_func(const r::api_status&, algo_server* ph) {
  ph->ml_error_handler();
}

BOOST_AUTO_TEST_CASE(typesafe_err_callback) {
  //start a http server that will receive events sent from the eventhub_client
  bool post_error = true;
  http_helper http_server;
  http_server.on_initialize(U("http://localhost:8080"),post_error);

  //create a simple ds configuration
  auto config = cfg::create_from_json(JSON_CFG);
  config.set(r::name::EH_TEST, "true");

  ////////////////////////////////////////////////////////////////////
  //// Following mismatched object type is prevented by the compiler
  //   wrong_class mismatch;
  //   live_model ds2(config, algo_error_func, &mismatch);
  ////////////////////////////////////////////////////////////////////

  algo_server the_server;
  //create a ds live_model, and initialize with configuration
  r::live_model ds(config,algo_error_func,&the_server);
  
  ds.init(nullptr);

  const char*  uuid = "uuid";

  r::ranking_response response;
  BOOST_CHECK_EQUAL(the_server._err_count, 0);
  // request ranking
  BOOST_CHECK_EQUAL(ds.choose_rank(uuid, JSON_CONTEXT, response), r::error_code::success);
  //wait until the timeout triggers and error callback is fired
  std::this_thread::sleep_for(std::chrono::milliseconds(1500));
  BOOST_CHECK_GT(the_server._err_count, 1);

  //stop the http server
  http_server.on_shutdown();
}
