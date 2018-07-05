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

BOOST_AUTO_TEST_CASE(live_model_ranking_request)
{
	//start a http server that will receive events sent from the eventhub_client
	http_helper http_server;
	http_server.on_initialize(U("http://localhost:8080"));

	//create a simple ds configuration
	auto config = reinforcement_learning::utility::config::init_from_json(R"({"eventhub_host":"localhost:8080"})");

	//create the ds live_model, and initialize it with the config
	reinforcement_learning::live_model ds(config);
  ds.init(nullptr);

  auto uuid = "uuid";
  auto context = "context";
  auto invalid_uuid = "";
  auto invalid_context = "";

  reinforcement_learning::ranking_response response;

	// request ranking
	BOOST_CHECK_EQUAL(ds.choose_rank(uuid, context, response), reinforcement_learning::error_code::success);

	//check expected returned codes
	BOOST_CHECK_EQUAL(ds.choose_rank(invalid_uuid, context, response), reinforcement_learning::error_code::invalid_argument);//invalid uuid
	BOOST_CHECK_EQUAL(ds.choose_rank(uuid, invalid_context, response), reinforcement_learning::error_code::invalid_argument);//invalid context

	//same tests now but with the api_status
  auto status = new reinforcement_learning::api_status();

	//invalid uuid
	ds.choose_rank(uuid, invalid_context, response, status);
	BOOST_CHECK_EQUAL(status->get_error_code(), reinforcement_learning::error_code::invalid_argument);

	//invalid context
	ds.choose_rank(invalid_uuid, context, response, status);
	BOOST_CHECK_EQUAL(status->get_error_code(), reinforcement_learning::error_code::invalid_argument);
	
	//valid request => status is reset
  reinforcement_learning::api_status::try_update(status, -42, "hello");
	ds.choose_rank(uuid, context, response, status);
	BOOST_CHECK_EQUAL(status->get_error_code(), 0);
	BOOST_CHECK_EQUAL(status->get_error_msg(), "");

	//stop the http server
	http_server.on_shutdown();

	delete status;
}

BOOST_AUTO_TEST_CASE(live_model_reward)
{
	//start a http server that will receive events sent from the eventhub_client
	http_helper http_server;
	http_server.on_initialize(U("http://localhost:8080"));

	//create a simple ds configuration
	auto config = reinforcement_learning::utility::config::init_from_json(R"({"eventhub_host":"localhost:8080"})");
  config.set("local_eventhub_test", "true");  

	//create a ds live_model, and initialize with configuration
  reinforcement_learning::live_model ds(config);
  ds.init(nullptr);

	const char*  uuid = "uuid";
	const char*  reward = "reward";
	const char*  invalid_uuid = "";
	const char*  invalid_reward = "";

	// report reward
	BOOST_CHECK_EQUAL(ds.report_outcome(uuid, reward), reinforcement_learning::error_code::success);

	// check expected returned codes
	BOOST_CHECK_EQUAL(ds.report_outcome(invalid_uuid, reward), reinforcement_learning::error_code::invalid_argument);//invalid uuid
	BOOST_CHECK_EQUAL(ds.report_outcome(uuid, invalid_reward), reinforcement_learning::error_code::invalid_argument);//invalid reward

	//check api_status content when errors are returned
	reinforcement_learning::api_status* status = new reinforcement_learning::api_status();

	//invalid uuid
	ds.report_outcome(invalid_uuid, reward, status);
	BOOST_CHECK_EQUAL(status->get_error_code(), reinforcement_learning::error_code::invalid_argument);

	//invalid context
	ds.report_outcome(uuid, invalid_reward, status);
	BOOST_CHECK_EQUAL(status->get_error_code(), reinforcement_learning::error_code::invalid_argument);
	
	//valid request => status is not modified
  reinforcement_learning::api_status::try_update(status, -42, "hello");
  ds.report_outcome(uuid, reward, status);
	BOOST_CHECK_EQUAL(status->get_error_code(), 0);
	BOOST_CHECK_EQUAL(status->get_error_msg(), "");

	//stop the http server
	http_server.on_shutdown();

	delete status;
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
  auto config = r::utility::config::init_from_json(R"({"eventhub_host":"localhost:8080"})");
  config.set("local_eventhub_test", "true");

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
  const char*  context = "abcdefghijklmnopqrstuvwxyz;;abcdefghijklmnopqrstuvwxyz";

  r::ranking_response response;
  BOOST_CHECK_EQUAL(the_server._err_count, 0);
  // request ranking
  BOOST_CHECK_EQUAL(ds.choose_rank(uuid, context, response), r::error_code::success);
  //wait until the timeout triggers and error callback is fired
  std::this_thread::sleep_for(std::chrono::milliseconds(1500));
  BOOST_CHECK_EQUAL(the_server._err_count, 1);

  //stop the http server
  http_server.on_shutdown();
}
