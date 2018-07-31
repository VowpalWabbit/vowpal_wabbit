#include "test_loop.h"

#include "config_utility.h"
#include <fstream>
#include <iostream>
#include <thread>

namespace r = reinforcement_learning;
namespace u = r::utility;
namespace cfg = u::config;
namespace err = r::error_code;
namespace po = boost::program_options;

test_loop::test_loop(const boost::program_options::variables_map& vm) 
  : threads(vm["threads"].as<unsigned int>())
  , examples(vm["examples"].as<unsigned int>())
  , json_config(vm["json_config"].as<std::string>())
  , test_inputs(vm["experiment_name"].as<std::string>(), threads, examples, vm["actions"].as<unsigned int>(), vm.count("float_reward") > 0)
  , is_perf(vm.count("perf") > 0)
{
  for (unsigned int i = 0; i < threads; ++i) {
    loggers.push_back(std::ofstream(vm["log_path"].as<std::string>() + "." + std::to_string(i), std::ofstream::out));
  }
}

bool test_loop::init() {
  r::api_status status;
  u::config_collection config;

  if (load_config_from_json(json_config, config, &status) != err::success) {
    std::cout << status.get_error_msg() << std::endl;
    return false;
  }

  rl = std::unique_ptr<r::live_model>(new r::live_model(config));
  if (rl->init(&status) != err::success) {
    std::cout << status.get_error_msg() << std::endl;
    return false;
  }

  return true;
}

int test_loop::load_config_from_json(const std::string& file_name,
  u::config_collection& cfgcoll,
  r::api_status* status) const {
  std::string config_str;
  RETURN_IF_FAIL(load_file(file_name, config_str));
  return cfg::create_from_json(config_str, cfgcoll, status);
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
  std::vector<std::thread> _threads;
  for (unsigned int i = 0; i < threads; ++i) {
    _threads.push_back(is_perf ? std::thread(&test_loop::perf_loop, this, i) : std::thread(&test_loop::validity_loop, this, i));
  }
  for (unsigned int i = 0; i < threads; ++i) {
    _threads[i].join();
  }
}

void test_loop::validity_loop(unsigned int thread_id) 
{
  r::ranking_response response;
  r::api_status status;

  for (unsigned int i = 0; i < examples; ++i) {
    if (rl->choose_rank(test_inputs.get_uuid(thread_id, i), test_inputs.get_context(thread_id, i), response, &status) != err::success) {
      std::cout << status.get_error_msg() << std::endl;
      continue;
    }
    
    if (test_inputs.is_rewarded(thread_id, i)) {
      if (test_inputs.report_outcome(rl.get(), thread_id, i, &status) != err::success) {
        std::cout << status.get_error_msg() << std::endl;
        continue;
      }
    }

    test_inputs.log(thread_id, i, response, loggers[thread_id]);
    response.clear();
  }
}

void test_loop::perf_loop(unsigned int thread_id)
{
  r::ranking_response response;
  r::api_status status;

  std::cout << "Perf test is started..." << std::endl;
  std::cout << "Choose_rank..." << std::endl;
  std::chrono::steady_clock::time_point choose_rank_start = std::chrono::steady_clock::now();
  for (unsigned int i = 0; i < examples; ++i) {
    if (rl->choose_rank(test_inputs.get_uuid(thread_id, i), test_inputs.get_context(thread_id, i), response, &status) != err::success) {
      std::cout << status.get_error_msg() << std::endl;
      continue;
    }
  }
  std::chrono::steady_clock::time_point choose_rank_end = std::chrono::steady_clock::now();

  std::cout << "Report_outcome..." << std::endl;
  std::chrono::steady_clock::time_point report_outcome_start = std::chrono::steady_clock::now();
  for (unsigned int i = 0; i < examples; ++i) {
    if (test_inputs.report_outcome(rl.get(), thread_id, i, &status) != err::success) {
      std::cout << status.get_error_msg() << std::endl;
      continue;
    }
    response.clear();
  }
  std::chrono::steady_clock::time_point report_outcome_end = std::chrono::steady_clock::now();

  loggers[thread_id] << thread_id << ": Choose_rank: " << (std::chrono::duration_cast<std::chrono::microseconds>(choose_rank_end - choose_rank_start).count()) / examples << std::endl;
  loggers[thread_id] << thread_id << ": Report outcome: " << (std::chrono::duration_cast<std::chrono::microseconds>(report_outcome_end - report_outcome_start).count()) / examples << std::endl;
}

