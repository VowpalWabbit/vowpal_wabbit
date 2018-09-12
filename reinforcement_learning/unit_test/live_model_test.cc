#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
#   define BOOST_TEST_MODULE Main
#endif

#include <thread>
#include <boost/test/unit_test.hpp>

#include "live_model.h"
#include "config_utility.h"
#include "api_status.h"
#include "ranking_response.h"
#include "err_constants.h"
#include "constants.h"
#include "sender.h"
#include "model_mgmt.h"
#include "str_util.h"

#include "mock_util.h"
#include "http_server/stdafx.h"
#include "http_server/http_server.h"

#include <fakeit/fakeit.hpp>

namespace r = reinforcement_learning;
namespace u = reinforcement_learning::utility;
namespace m = reinforcement_learning::model_management;
namespace err = reinforcement_learning::error_code;
namespace cfg = reinforcement_learning::utility::config;

using namespace fakeit;

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

BOOST_AUTO_TEST_CASE(live_model_ranking_request) {
  auto mock_sender = get_mock_sender();
  auto mock_data_transport = get_mock_data_transport();
  auto mock_model = get_mock_model();

  auto sender_factory = get_mock_sender_factory(mock_sender.get(), mock_sender.get());
  auto data_transport_factory = get_mock_data_transport_factory(mock_data_transport.get());
  auto model_factory = get_mock_model_factory(mock_model.get());

  //create a simple ds configuration
  u::configuration config;
  cfg::create_from_json(JSON_CFG, config);
  config.set(r::name::EH_TEST, "true");

  r::api_status status;

  //create the ds live_model, and initialize it with the config
  r::live_model ds(config, nullptr, nullptr, data_transport_factory.get(), model_factory.get(), sender_factory.get());
  BOOST_CHECK_EQUAL(ds.init(&status), err::success);

  const auto event_id = "event_id";
  const auto invalid_event_id = "";
  const auto invalid_context = "";

  r::ranking_response response;

  // request ranking
  BOOST_CHECK_EQUAL(ds.choose_rank(event_id, JSON_CONTEXT, response), err::success);

  // check expected returned codes
  BOOST_CHECK_EQUAL(ds.choose_rank(invalid_event_id, JSON_CONTEXT, response), err::invalid_argument); // invalid event_id
  BOOST_CHECK_EQUAL(ds.choose_rank(event_id, invalid_context, response), err::invalid_argument); // invalid context

  // invalid event_id
  ds.choose_rank(event_id, invalid_context, response, &status);
  BOOST_CHECK_EQUAL(status.get_error_code(), err::invalid_argument);

  //invalid context
  ds.choose_rank(invalid_event_id, JSON_CONTEXT, response, &status);
  BOOST_CHECK_EQUAL(status.get_error_code(), err::invalid_argument);

  //valid request => status is reset
  r::api_status::try_update(&status, -42, "hello");
  ds.choose_rank(event_id, JSON_CONTEXT, response, &status);
  BOOST_CHECK_EQUAL(status.get_error_code(), 0);
  BOOST_CHECK_EQUAL(status.get_error_msg(), "");
}

BOOST_AUTO_TEST_CASE(live_model_outcome) {
  auto mock_sender = get_mock_sender();
  auto mock_data_transport = get_mock_data_transport();
  auto mock_model = get_mock_model();

  auto sender_factory = get_mock_sender_factory(mock_sender.get(), mock_sender.get());
  auto data_transport_factory = get_mock_data_transport_factory(mock_data_transport.get());
  auto model_factory = get_mock_model_factory(mock_model.get());

  //create a simple ds configuration
  u::configuration config;
  cfg::create_from_json(JSON_CFG, config);
  config.set(r::name::EH_TEST, "true");

  //create a ds live_model, and initialize with configuration
  r::live_model ds(config, nullptr, nullptr, data_transport_factory.get(), model_factory.get(), sender_factory.get());

  //check api_status content when errors are returned
  r::api_status status;

  BOOST_CHECK_EQUAL(ds.init(&status), err::success);
  BOOST_CHECK_EQUAL(status.get_error_code(), err::success);
  BOOST_CHECK_EQUAL(status.get_error_msg(), "");

  const auto event_id = "event_id";
  const auto  outcome = "outcome";
  const auto  invalid_event_id = "";
  const auto  invalid_outcome = "";

  // report outcome
  const auto scode = ds.report_outcome(event_id, outcome, &status);
  BOOST_CHECK_EQUAL(scode, err::success);
  BOOST_CHECK_EQUAL(status.get_error_msg(), "");

  // check expected returned codes
  BOOST_CHECK_EQUAL(ds.report_outcome(invalid_event_id, outcome), err::invalid_argument);//invalid event_id
  BOOST_CHECK_EQUAL(ds.report_outcome(event_id, invalid_outcome), err::invalid_argument);//invalid outcome

  //invalid event_id
  ds.report_outcome(invalid_event_id, outcome, &status);
  BOOST_CHECK_EQUAL(status.get_error_code(), reinforcement_learning::error_code::invalid_argument);

  //invalid context
  ds.report_outcome(event_id, invalid_outcome, &status);
  BOOST_CHECK_EQUAL(status.get_error_code(), reinforcement_learning::error_code::invalid_argument);

  //valid request => status is not modified
  r::api_status::try_update(&status, -42, "hello");
  ds.report_outcome(event_id, outcome, &status);
  BOOST_CHECK_EQUAL(status.get_error_code(), err::success);
  BOOST_CHECK_EQUAL(status.get_error_msg(), "");
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
  BOOST_CHECK(http_server.on_initialize(U("http://localhost:8080"),post_error));

  //create a simple ds configuration
  u::configuration config;
  auto const status = cfg::create_from_json(JSON_CFG,config);
  BOOST_CHECK_EQUAL(status, r::error_code::success);
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

  const char* event_id = "event_id";

  r::ranking_response response;
  BOOST_CHECK_EQUAL(the_server._err_count, 0);
  // request ranking
  BOOST_CHECK_EQUAL(ds.choose_rank(event_id, JSON_CONTEXT, response), r::error_code::success);
  //wait until the timeout triggers and error callback is fired
  std::this_thread::sleep_for(std::chrono::milliseconds(1500));
  BOOST_CHECK_GT(the_server._err_count, 1);
}

BOOST_AUTO_TEST_CASE(live_model_mocks) {
  std::vector<std::string> recorded;
  auto mock_sender = get_mock_sender(recorded);
  auto mock_data_transport = get_mock_data_transport();
  auto mock_model = get_mock_model();

  auto sender_factory = get_mock_sender_factory(mock_sender.get(), mock_sender.get());
  auto data_transport_factory = get_mock_data_transport_factory(mock_data_transport.get());
  auto model_factory = get_mock_model_factory(mock_model.get());

  u::configuration config;
  cfg::create_from_json(JSON_CFG, config);
  config.set(r::name::EH_TEST, "true");
  {
    r::live_model model(config, nullptr, nullptr, data_transport_factory.get(), model_factory.get(), sender_factory.get());

    r::api_status status;
    BOOST_CHECK_EQUAL(model.init(&status), err::success);

    const auto event_id = "event_id";
    r::ranking_response response;

    BOOST_CHECK_EQUAL(model.choose_rank(event_id, JSON_CONTEXT, response), err::success);
    BOOST_CHECK_EQUAL(model.report_outcome(event_id, 1.0), err::success);

    Verify(Method((*mock_sender), init)).Exactly(2);
  }
  BOOST_CHECK_EQUAL(recorded.size(), 2);
}

BOOST_AUTO_TEST_CASE(live_model_logger_receive_data) {
  std::vector<std::string> recorded_observations;
  auto mock_observation_sender = get_mock_sender(recorded_observations);

  std::vector<std::string> recorded_interactions;
  auto mock_interaction_sender = get_mock_sender(recorded_interactions);

  auto mock_data_transport = get_mock_data_transport();
  auto mock_model = get_mock_model();

  auto logger_factory = get_mock_sender_factory(mock_observation_sender.get(), mock_interaction_sender.get());
  auto data_transport_factory = get_mock_data_transport_factory(mock_data_transport.get());
  auto model_factory = get_mock_model_factory(mock_model.get());

  u::configuration config;
  cfg::create_from_json(JSON_CFG, config);
  config.set(r::name::EH_TEST, "true");

  auto const version_number = "1";

  auto const event_id_1 = "event_id";
  auto const event_id_2 = "event_id_2";

  auto const expected_interaction_1 = u::concat(R"({"Version":")", version_number, R"(","EventId":")", event_id_1, R"(","a":[1,2],"c":)", JSON_CONTEXT, R"(,"p":[0.500000,0.500000],"VWState":{"m":"N/A"}})");
  auto const expected_observation_1 = u::concat(R"({"EventId":")", event_id_1, R"(","v":1.000000})");

  auto const expected_interaction_2 = u::concat(R"({"Version":")", version_number, R"(","EventId":")", event_id_2, R"(","a":[1,2],"c":)", JSON_CONTEXT, R"(,"p":[0.500000,0.500000],"VWState":{"m":"N/A"}})");
  auto const expected_observation_2 = u::concat(R"({"EventId":")", event_id_2, R"(","v":1.000000})");
  auto const num_iterations = 5;

  std::string expected_interactions;
  std::string expected_observations;
  {
    r::live_model model(config, nullptr, nullptr, data_transport_factory.get(), model_factory.get(), logger_factory.get());

    r::api_status status;
    BOOST_CHECK_EQUAL(model.init(&status), err::success);

    r::ranking_response response;
    for (auto i = 0; i < num_iterations; i++) {
      BOOST_CHECK_EQUAL(model.choose_rank(event_id_1, JSON_CONTEXT, response), err::success);
      BOOST_CHECK_EQUAL(model.report_outcome(event_id_1, 1.0), err::success);
      expected_interactions += expected_interaction_1 + '\n';
      expected_observations += expected_observation_1 + '\n';

      BOOST_CHECK_EQUAL(model.choose_rank(event_id_2, JSON_CONTEXT, response), err::success);
      BOOST_CHECK_EQUAL(model.report_outcome(event_id_2, 1.0), err::success);
      expected_interactions += expected_interaction_2;
      expected_observations += expected_observation_2;
      if (i + 1 < num_iterations) {
        expected_interactions += '\n';
        expected_observations += '\n';
      }
    }

    Verify(Method((*mock_observation_sender), init)).Exactly(1);
    Verify(Method((*mock_interaction_sender), init)).Exactly(1);
  }
  std::string recorded_interactions_all;
  for (auto i = 0; i < recorded_interactions.size(); ++i) {
    recorded_interactions_all += recorded_interactions[i];
    if (i + 1 < recorded_interactions.size()) {
      recorded_interactions_all += '\n';
    }
  }

  std::string recorded_observations_all;
  for (auto i = 0; i < recorded_interactions.size(); ++i) {
    recorded_observations_all += recorded_observations[i];
    if (i + 1 < recorded_observations.size()) {
      recorded_observations_all += '\n';
    }
  }

  BOOST_CHECK_EQUAL(recorded_interactions_all, expected_interactions);
  BOOST_CHECK_EQUAL(recorded_observations_all, expected_observations);
}

