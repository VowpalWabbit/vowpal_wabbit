#pragma once
#include "sender.h"
#include "configuration.h"

#include <boost/program_options.hpp>

class test_loop {
public:
  test_loop(const boost::program_options::variables_map& vm);
  bool init();
  void run();

private:
  int load_file(const std::string& file_name, std::string& config_str) const;
  int load_config_from_json(const std::string& file_name,
    reinforcement_learning::utility::configuration& config,
    reinforcement_learning::api_status* status) const;
  std::string get_message(size_t i) const;
  void init_messages();

private:
  const size_t _message_size;
  const size_t _message_count;
  const size_t _threads;
  const std::string _json_config;
  std::unique_ptr<reinforcement_learning::i_sender> _sender;
  std::string _message;
};
