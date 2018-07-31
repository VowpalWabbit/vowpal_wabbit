#pragma once
#include "test_data_provider.h"
#include "live_model.h"

#include <boost/program_options.hpp>

class test_loop {
public:
  test_loop(const boost::program_options::variables_map& vm);
  bool init();
  void run();

private:
  int load_file(const std::string& file_name, std::string& config_str) const;
  int load_config_from_json(const std::string& file_name,
    reinforcement_learning::utility::config_collection& cfgcoll,
    reinforcement_learning::api_status* status) const;

  void validity_loop(unsigned int thread_id);
  void perf_loop(unsigned int thread_id);

private:
  const unsigned int threads;
  const unsigned int examples;
  const std::string json_config;
  test_data_provider test_inputs;
  const bool is_perf;

  std::vector<std::ofstream> loggers;
  std::unique_ptr<reinforcement_learning::live_model> rl;
};
