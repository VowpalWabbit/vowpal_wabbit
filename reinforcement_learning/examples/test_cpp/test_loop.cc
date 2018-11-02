#include "test_loop.h"
#include "ranking_event.h"
#include "utility/data_buffer.h"
#include "config_utility.h"
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
  : threads(vm["threads"].as<size_t>())
  , examples(vm["examples"].as<size_t>())
  , experiment_name(generate_experiment_name(vm["experiment_name"].as<std::string>(), threads, examples, vm["features"].as<size_t>(), vm["actions"].as<size_t>()))
  , json_config(vm["json_config"].as<std::string>())
  , test_inputs(experiment_name, threads, examples, vm["features"].as<size_t>(), vm["actions"].as<size_t>(), vm.count("float_outcome") > 0)
  , is_perf(vm.count("perf") > 0)
  , sleep_interval(vm["sleep"].as<size_t>())
{
  for (size_t i = 0; i < threads; ++i) {
    loggers.push_back(std::ofstream(experiment_name + "." + std::to_string(i), std::ofstream::out));
  }
}

void _on_error(const reinforcement_learning::api_status& status, void* nothing) {
  std::cerr << status.get_error_msg() << std::endl;
}

bool test_loop::init() {
  r::api_status status;
  u::configuration config;

  if (load_config_from_json(json_config, config, &status) != err::success) {
    std::cout << status.get_error_msg() << std::endl;
    return false;
  }

  rl = std::unique_ptr<r::live_model>(new r::live_model(config, _on_error, nullptr));
  if (rl->init(&status) != err::success) {
    std::cout << status.get_error_msg() << std::endl;
    return false;
  }

  return true;
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

std::string test_loop::generate_experiment_name(const std::string& experiment_name_base, size_t threads, 
	size_t examples, size_t features, size_t actions)
{
  return experiment_name_base + "-t" + std::to_string(threads) + "-n" + std::to_string(examples) 
	  + "-f" + std::to_string(features) + "-a" + std::to_string(actions);
}

void test_loop::run() {
  std::vector<std::thread> _threads;
  for (size_t i = 0; i < threads; ++i) {
    _threads.push_back(is_perf ? std::thread(&test_loop::perf_loop, this, i) : std::thread(&test_loop::validity_loop, this, i));
  }
  for (size_t i = 0; i < threads; ++i) {
    _threads[i].join();
  }
}

void test_loop::validity_loop(size_t thread_id) 
{
  r::ranking_response response;
  r::api_status status;
  auto const step = examples / 100;
  for (size_t i = 0; i < examples; ++i) {
    if (step > 0 && i % step == 0 && thread_id == 0) std::cout << "\r" << (i / step) << "%";

    if (rl->choose_rank(test_inputs.get_event_id(thread_id, i), test_inputs.get_context(thread_id, i), response, &status) != err::success) {
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
    
    if (sleep_interval > 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds(sleep_interval));
    }
  }
  std::cout << std::endl;
}

void test_loop::perf_loop(size_t thread_id)
{
  r::ranking_response response;
  r::api_status status;
  std::cout << "Warmup..." << std::endl;
  size_t choose_rank_size = 0;
  {
    r::utility::data_buffer buffer;
    const std::string warmup_id = "_warmup_" + std::string(test_inputs.get_event_id(0, 0));
    if (rl->choose_rank(warmup_id.c_str(), test_inputs.get_context(0, 0), response, &status) != err::success) {
      std::cout << "Warmup is failed. " << status.get_error_msg() << std::endl;
      return;
    }
    auto choose_rank_event = r::ranking_event::choose_rank(buffer, warmup_id.c_str(), test_inputs.get_context(0, 0), r::action_flags::DEFAULT, response);
    choose_rank_size = choose_rank_event.size();
    std::cout << "Choose rank size: " << choose_rank_size << std::endl;
  }  

  std::cout << "Perf test is started..." << std::endl;
  std::cout << "Choose_rank..." << std::endl;
  auto const step = examples / 100;
  const auto choose_rank_start = chrono::high_resolution_clock::now();
  for (size_t i = 0; i < examples; ++i) {
    if (step > 0 && i % step == 0 && thread_id == 0) std::cout << "\r" << (i / step) << "%";

    if (rl->choose_rank(test_inputs.get_event_id(thread_id, i), test_inputs.get_context(thread_id, i), response, &status) != err::success) {
      std::cout << status.get_error_msg() << std::endl;
      continue;
    }
  }
  std::cout << std::endl;

  const auto choose_rank_end = chrono::high_resolution_clock::now();

  const auto choose_rank_perf = (chrono::duration_cast<chrono::microseconds>(choose_rank_end - choose_rank_start).count()) / examples;
  loggers[thread_id] << thread_id << ": Choose_rank: " << choose_rank_perf << " microseconds" << std::endl;
}

