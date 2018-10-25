#include "test_loop.h"

#include "constants.h"
#include "config_utility.h"
#include "factory_resolver.h"
#include <fstream>
#include <iostream>
#include <thread>

namespace r = reinforcement_learning;
namespace u = r::utility;
namespace cfg = u::config;
namespace err = r::error_code;
namespace po = boost::program_options;
namespace chrono = std::chrono;

test_loop::test_loop(const boost::program_options::variables_map& vm) 
  : _message_size(vm["message_size"].as<size_t>())
  , _message_count(vm["message_count"].as<size_t>())
  , _threads(vm["threads"].as<size_t>())
  , _json_config(vm["json_config"].as<std::string>())
{
}

bool test_loop::init() {
  std::cout << "Initializing...." << std::endl;
  r::api_status status;
  u::configuration config;

  if (load_config_from_json(_json_config, config, &status) != err::success) {
    std::cout << status.get_error_msg() << std::endl;
    return false;
  }
  config.set(r::name::INTERACTION_EH_TASKS_LIMIT, std::to_string(_threads).c_str());
  const auto sender_impl = config.get(r::name::INTERACTION_SENDER_IMPLEMENTATION, r::value::INTERACTION_EH_SENDER);
  r::i_sender* sender;
  if (r::sender_factory.create(&sender, sender_impl, config, &status) != r::error_code::success) {
    std::cout << status.get_error_msg() << std::endl;
    return false;
  }
  _sender.reset(sender);

  if (_sender->init(&status) != r::error_code::success) {
    std::cout << status.get_error_msg() << std::endl;
    return false;
  }
  std::cout << "Done" << std::endl;
  return true;
}

void test_loop::init_messages() {
  std::vector<char> buffer(_message_size * 1024, '0');
  _message = std::string(&buffer[0], _message_size);
}

int test_loop::load_config_from_json(const std::string& file_name,
  u::configuration& config,
  r::api_status* status) const {
  std::string config_str;
  RETURN_IF_FAIL(load_file(file_name, config_str));
  return cfg::create_from_json(config_str, config, nullptr, status);
}

int test_loop::load_file(const std::string& file_name, std::string& config_str) const {
  std::ifstream fs;
  fs.open(file_name);
  if (!fs.good()) return err::invalid_argument;
  std::stringstream buffer;
  buffer << fs.rdbuf();
  config_str = buffer.str();
  return err::success;
}

void test_loop::run() {
  std::cout << "Testing...." << std::endl;
  const auto start = chrono::high_resolution_clock::now();
  auto const step = _message_count / 100;
  for (size_t i = 0; i < _message_count; ++i) {
    if (step > 0 && i % step == 0) std::cout << "\r" << (i / step) << "%";
    auto message = get_message(i);
    _sender->send_string(std::move(message));
  }
  std::cout << std::endl << "Done" << std::endl << std::endl;

  const auto end = chrono::high_resolution_clock::now();
  const auto duration = chrono::duration_cast<chrono::microseconds>(end - start).count();
  const auto Kb = _message_size * _message_count;
  std::cout << "Throughput: " << ((float)(Kb * 1000000)) / duration << " Kb/s" << std::endl;
}

std::string test_loop::get_message(size_t i) const
{
  return _message;
}

