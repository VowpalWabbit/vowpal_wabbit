#pragma once
#include "live_model.h"
#include "ranking_response.h"

#include <fstream>
#include <string>
#include <vector>

class test_data_provider {
public:
  test_data_provider(const std::string& experiment_name, unsigned int threads, unsigned int examples, unsigned int actions, bool _is_float_reward);

  const char* get_uuid(unsigned int thread_id, unsigned int example_id) const;
  const char* get_context(unsigned int thread_id, unsigned int example_id) const;

  float get_reward(unsigned int thread_id, unsigned int example_id) const;
  const char* get_reward_json(unsigned int thread_id, unsigned int example_id) const;

  bool is_rewarded(unsigned int thread_id, unsigned int example_id) const;

  int report_outcome(reinforcement_learning::live_model* rl, unsigned int thread_id, unsigned int example_id, reinforcement_learning::api_status* status) const;

  void log(unsigned int thread_id, unsigned int example_id, const reinforcement_learning::ranking_response& response, std::ostream& logger) const;

private:
  std::string create_uuid(const std::string& experiment_name, unsigned int thread_id, unsigned int example_id) const;
  std::string get_action_features(unsigned int count) const;
  std::string create_features(unsigned int thread_id, unsigned int example_id) const;
  std::string create_context_json(const std::string& cntxt, const std::string& action) const;
  std::string create_json_reward(unsigned int thread_id, unsigned int example_id) const;

private:
  std::vector<std::vector<std::string>> uuids;
  std::vector<std::vector<std::string>> contexts;
  std::vector<std::vector<std::string>> rewards;
  std::vector<std::vector<bool>> reward_flag;
  bool is_float_reward;
};