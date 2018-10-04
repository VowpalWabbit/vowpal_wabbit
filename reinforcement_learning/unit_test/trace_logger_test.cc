#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
#   define BOOST_TEST_MODULE Main
#endif

#include <boost/test/unit_test.hpp>
#include <fakeit/fakeit.hpp>
#include "mock_util.h"
#include "live_model.h"
#include "config_utility.h"
#include "constants.h"
#include "model_mgmt.h"
#include "api_status.h"
#include "err_constants.h"
#include "console_tracer.h"

namespace r = reinforcement_learning;
namespace u = reinforcement_learning::utility;
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

struct vector_tracer : r::i_trace {
  void log(int log_level, const std::string& msg) override {
    data.emplace_back(msg);
  }
  std::vector<std::string> data;
};

vector_tracer* the_tracer = new vector_tracer();
int vector_trace_create(r::i_trace** retval, const u::configuration&, r::i_trace* trace_logger, r::api_status* status) {
  *retval = the_tracer;
  return err::success;
}

BOOST_AUTO_TEST_CASE(test_trace_logging) {
  auto mock_logger = get_mock_sender();
  auto mock_data_transport = get_mock_data_transport();
  auto mock_model = get_mock_model();

  auto logger_factory = get_mock_sender_factory(mock_logger.get(), mock_logger.get());
  auto data_transport_factory = get_mock_data_transport_factory(mock_data_transport.get());
  auto model_factory = get_mock_model_factory(mock_model.get());

  //create a simple ds configuration
  u::configuration config;
  cfg::create_from_json(JSON_CFG, config);
  config.set(r::name::EH_TEST, "true");


  config.set(r::name::TRACE_LOG_IMPLEMENTATION, "VectorTracer");
  r::trace_logger_factory.register_type("VectorTracer", vector_trace_create);

  r::api_status status;

  //create the ds live_model, and initialize it with the config
  r::live_model ds(config, nullptr, nullptr, &r::trace_logger_factory, data_transport_factory.get(), model_factory.get(), logger_factory.get());
  BOOST_CHECK_EQUAL(ds.init(&status), err::success);
  BOOST_CHECK_EQUAL(the_tracer->data[0], "API Tracing initialized");
}

BOOST_AUTO_TEST_CASE(test_console_logging) {
  reinforcement_learning::console_tracer trace;
  trace.log(0, "Test message");
}
