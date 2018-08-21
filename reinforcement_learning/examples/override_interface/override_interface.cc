/**
 * @brief RL inference API example showing custom overriden interface
 *
 * @file override_interface.cc
 * @author Jack Gerrits et al
 * @date 2018-08-20
 */
#include <iostream>
#include <fstream>

#include "config_utility.h"
#include "live_model.h"
#include "factory_resolver.h"
#include "constants.h"
#include "logger.h"

// Namespace manipulation for brevity
namespace r = reinforcement_learning;
namespace u = r::utility;
namespace cfg = u::config;
namespace err = r::error_code;

// Custom implementations must inherit from the respective i_* abstract class.
class ostream_logger : public r::i_logger {
public:
  ostream_logger(std::ostream& stream)
    : _stream(stream)
  {}

  virtual int init(r::api_status* status) override {
    return r::error_code::success;
  }

protected:
  virtual int v_append(std::string& data, r::api_status* status) override {
    _stream << data << std::endl;
    return r::error_code::success;
  }

private:
  std::ostream& _stream;
};

//! Load contents of file into a string
int load_file(const std::string& file_name, std::string& config_str) {
  std::ifstream fs;
  fs.open(file_name);
  if (!fs.good())
    return reinforcement_learning::error_code::invalid_argument;
  std::stringstream buffer;
  buffer << fs.rdbuf();
  config_str = buffer.str();
  return r::error_code::success;
}

//! Load config from json file
int load_config_from_json(const std::string& file_name, u::configuration& cfg) {
  std::string config_str;
  RETURN_IF_FAIL(load_file(file_name, config_str));
  RETURN_IF_FAIL(cfg::create_from_json(config_str, cfg));
  return r::error_code::success;
}

int main() {
  u::configuration config;
  if( load_config_from_json("client.json", config) != err::success ) {
    std::cout << "Unable to Load file: client.json" << std::endl;
    return -1;
  }
  r::api_status status;

  // Define a create function to be used in the factory.
  auto const create_ostream_logger_fn =
    [](r::i_logger** retval, const u::configuration&, r::error_callback_fn*, r::api_status*) {
    *retval = new ostream_logger(std::cout);
    return err::success;
  };

  // Create a local factory and register the create function with it usinf the corresponding implementation keys defined in constants.h
  r::logger_factory_t stdout_logger_factory;
  stdout_logger_factory.register_type(r::value::OBSERVATION_EH_LOGGER, create_ostream_logger_fn);
  stdout_logger_factory.register_type(r::value::INTERACTION_EH_LOGGER, create_ostream_logger_fn);

  // Default factories defined in factory_resolver.h are passed as well as the custom stdout logger.
  r::live_model rl(config, nullptr, nullptr, &r::data_transport_factory, &r::model_factory, &stdout_logger_factory);

  if( rl.init(&status) != err::success ) {
    std::cout << status.get_error_msg() << std::endl;
    return -1;
  }

  r::ranking_response response;

  char const * const event_id = "event_id";
  char const * const context =
     R"({"GUser":{"id":"a","major":"eng","hobby":"hiking"},"_multi":[ { "TAction":{"a1":"f1"} },{"TAction":{"a2":"f2"}}]})";
  float outcome  = 1.0f;

  if ( rl.choose_rank(event_id, context, response, &status) != err::success ) {
    std::cout << status.get_error_msg() << std::endl;
    return -1;
  }

  if( rl.report_outcome(event_id, outcome, &status) != err::success ) {
    std::cout << status.get_error_msg() << std::endl;
    return -1;
  }

  return 0;
}
