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
#include "logger.h"
#include "model_mgmt.h"

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
  //start a http server that will receive events sent from the eventhub_client
  http_helper http_server;
  BOOST_CHECK(http_server.on_initialize(U("http://localhost:8080")));
  r::api_status status;

  //create a simple ds configuration
  u::configuration config;
  cfg::create_from_json(JSON_CFG, config);
  config.set(r::name::EH_TEST, "true");

  //create the ds live_model, and initialize it with the config
  r::live_model ds(config);
  BOOST_CHECK_EQUAL(ds.init(&status), err::success);

  const auto event_id = "event_id";
  const auto invalid_event_id = "";
  const auto invalid_context = "";

  r::ranking_response response;

  // request ranking
  BOOST_CHECK_EQUAL(ds.choose_rank(event_id, JSON_CONTEXT, response), err::success);

  //check expected returned codes
  BOOST_CHECK_EQUAL(ds.choose_rank(invalid_event_id, JSON_CONTEXT, response), err::invalid_argument);//invalid event_id
  BOOST_CHECK_EQUAL(ds.choose_rank(event_id, invalid_context, response), err::invalid_argument);//invalid context

  //invalid event_id
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
  //start a http server that will receive events sent from the eventhub_client
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  http_helper http_server;
  BOOST_CHECK(http_server.on_initialize(U("http://localhost:8080")));
  std::this_thread::sleep_for(std::chrono::milliseconds(500));

  //create a simple ds configuration
  u::configuration config;
  cfg::create_from_json(JSON_CFG, config);
  config.set(r::name::EH_TEST, "true");

  //create a ds live_model, and initialize with configuration
  r::live_model ds(config);

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
  BOOST_CHECK_EQUAL(the_server._err_count, 0);

  ds.init(nullptr);

  const char* event_id = "event_id";

  r::ranking_response response;
  // request ranking
  BOOST_CHECK_EQUAL(ds.choose_rank(event_id, JSON_CONTEXT, response), r::error_code::success);
  //wait until the timeout triggers and error callback is fired
  std::this_thread::sleep_for(std::chrono::milliseconds(1500));
  BOOST_CHECK_GT(the_server._err_count, 1);
}

BOOST_AUTO_TEST_CASE(live_model_mocks) {
  Mock<r::i_logger> mock_logger;
  When(Method(mock_logger, init)).AlwaysReturn(err::success);
  When(Method(mock_logger, append)).AlwaysReturn(err::success);
  Fake(Dtor(mock_logger));

  Mock<m::i_data_transport> mock_data_transport;
  When(Method(mock_data_transport, get_data)).AlwaysReturn(err::success);
  Fake(Dtor(mock_data_transport));

  Mock<m::i_model> mock_model;
  When(Method(mock_model, update)).AlwaysReturn(err::success);
  When(Method(mock_model, choose_rank)).AlwaysReturn(err::success);
  Fake(Dtor(mock_model));

  r::logger_factory_t logger_factory;
  logger_factory.register_type(r::value::OBSERVATION_EH_LOGGER,
    [&mock_logger](r::i_logger** retval, const u::configuration&, reinforcement_learning::utility::watchdog&, r::error_callback_fn*, r::api_status*) { *retval = &mock_logger.get(); return err::success; });
  logger_factory.register_type(r::value::INTERACTION_EH_LOGGER,
    [&mock_logger](r::i_logger** retval, const u::configuration&, reinforcement_learning::utility::watchdog&, r::error_callback_fn*, r::api_status*) { *retval = &mock_logger.get(); return err::success; });

  r::data_transport_factory_t data_transport_factory;
  data_transport_factory.register_type(r::value::AZURE_STORAGE_BLOB,
    [&mock_data_transport](m::i_data_transport** retval, const u::configuration&, r::api_status*){*retval = &mock_data_transport.get(); return err::success;});
  r::model_factory_t model_factory;
  model_factory.register_type(r::value::VW,
    [&mock_model](m::i_model** retval, const u::configuration&, r::api_status*){*retval = &mock_model.get(); return err::success;});

  u::configuration config;
  cfg::create_from_json(JSON_CFG, config);
  config.set(r::name::EH_TEST, "true");

  r::live_model model(config, nullptr, nullptr, &data_transport_factory, &model_factory, &logger_factory);

  r::api_status status;
  BOOST_CHECK_EQUAL(model.init(&status), err::success);

  const auto event_id = "event_id";
  r::ranking_response response;

  BOOST_CHECK_EQUAL(model.choose_rank(event_id, JSON_CONTEXT, response), err::success);
  BOOST_CHECK_EQUAL(model.report_outcome(event_id, 1.0), err::success);

  Verify(Method(mock_logger, init)).Exactly(2);
  Verify(Method(mock_logger, append)).Exactly(2);
}
