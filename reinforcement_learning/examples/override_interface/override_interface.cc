/**
 * @brief model inference API example showing custom overriden interface
 *
 * @file override_interface.cc
 * @author Jack Gerrits et al
 * @date 2018-08-20
 */
#include <iostream>
#include <fstream>
#include <mutex>

#include "config_utility.h"
#include "live_model.h"
#include "factory_resolver.h"
#include "constants.h"
#include "sender.h"

// Namespace manipulation for brevity
namespace r = reinforcement_learning;
namespace u = r::utility;
namespace err = r::error_code;

// Custom implementations must inherit from the respective i_* abstract class.
class ostream_sender : public r::i_sender {
public:
  ostream_sender(std::ostream& stream, std::mutex& mutex)
    : _stream(stream),
      _mutex(mutex) {}

  virtual int init(r::api_status* status) override {
    return err::success;
  }

protected:
  virtual int v_send(const std::string& data, r::api_status* status) override {
    std::lock_guard<std::mutex> lock(_mutex);
    _stream << data << std::endl;
    return err::success;
  }

private:
  std::ostream& _stream;
  std::mutex& _mutex;
};

// Load contents of file into a string
int load_file(const std::string& file_name, std::string& config_str) {
  std::ifstream fs;
  fs.open(file_name);
  if (!fs.good())
    return err::invalid_argument;
  std::stringstream buffer;
  buffer << fs.rdbuf();
  config_str = buffer.str();
  return err::success;
}

// Load config from json file
int load_config_from_json(const std::string& file_name, u::configuration& cfg) {
  std::string config_str;
  RETURN_IF_FAIL(load_file(file_name, config_str));
  RETURN_IF_FAIL(u::config::create_from_json(config_str, cfg));
  return err::success;
}

int main() {
  u::configuration config;
  if (load_config_from_json("client.json", config) != err::success) {
    std::cout << "Unable to Load file: client.json" << std::endl;
    return -1;
  }
  r::api_status status;

  // Used to synchronize writes to cout across multiple logger.
  std::mutex cout_mutex;

  // Define a create function to be used in the factory.
  auto const create_ostream_sender_fn =
    [&](r::i_sender** retval, const u::configuration&, r::api_status*) {
    *retval = new ostream_sender(std::cout, cout_mutex);
    return err::success;
  };

  // Create a local factory and register the create function with it using the corresponding implementation keys defined in constants.h
  r::sender_factory_t stdout_logger_factory;
  stdout_logger_factory.register_type(r::value::OBSERVATION_EH_SENDER, create_ostream_sender_fn);
  stdout_logger_factory.register_type(r::value::INTERACTION_EH_SENDER, create_ostream_sender_fn);

  // Default factories defined in factory_resolver.h are passed as well as the custom stdout logger.
  r::live_model model(config, nullptr, nullptr, &r::data_transport_factory, &r::model_factory, &stdout_logger_factory);

  if (model.init(&status) != err::success) {
    std::cout << status.get_error_msg() << std::endl;
    return -1;
  }

  r::ranking_response response;

  char const* const event_id = "event_id";
  char const* const context =
    R"({"GUser":{"id":"a","major":"eng","hobby":"hiking"},"_multi":[ { "TAction":{"a1":"f1"} },{"TAction":{"a2":"f2"}}]})";
  float outcome = 1.0f;

  if (model.choose_rank(event_id, context, response, &status) != err::success) {
    std::cout << status.get_error_msg() << std::endl;
    return -1;
  }

  if (model.report_outcome(event_id, outcome, &status) != err::success) {
    std::cout << status.get_error_msg() << std::endl;
    return -1;
  }

  return 0;
}
