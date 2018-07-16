/**
 * @brief Basic usage example
 * 
 * @file basic_usage_cpp.cc
 * @author your name
 * @date 2018-07-15
 */
#include "basic_usage_cpp.h"

/**
 * @brief Basic API usage example
 * 
 * @return int Error code 
 */
int api_example() {
  u::config_collection config;
  RETURN_ON_ERROR_STR(load_config_from_json("client.json", config), 
    "Unable to Load file: client.json");

  // api_status is an optional argument used to return status of all API calls
  r::api_status status;

  // Create an interface to reinforcement learning loop using config
  auto rl = new r::live_model(config);
  RETURN_ON_ERROR(rl->init(&status), status);

  // Response class
  r::ranking_response response;

  // Choose an action
  RETURN_ON_ERROR(rl->choose_rank(uuid, context, response, &status), status);

  size_t choosen_action;
  RETURN_ON_ERROR(response.get_choosen_action_id(choosen_action), status);

  // Use the response
  std::cout << "Chosen action id is: " << choosen_action << std::endl;

  // Report reward recieved
  RETURN_ON_ERROR(rl->report_outcome(uuid, reward, &status), status);

  return reinforcement_learning::error_code::success;
}

int main() {
  const auto retcode = api_example();
  return retcode;
}

// Helper methods

// Load config from json file
int load_config_from_json(const std::string& file_name, u::config_collection& cfgcoll) {
  std::string config_str;
  // Load contents of config file into a string
  const auto scode = load_file(file_name, config_str);
  if ( scode != 0 ) return scode;
  // Use library supplied convinence method to parse json and build config object
  return cfg::create_from_json(config_str, cfgcoll);
}

// Load contents of file into a string
int load_file(const std::string& file_name, std::string& config_str) {
  std::ifstream fs;
  fs.open(file_name);
  if ( !fs.good() )
    return reinforcement_learning::error_code::invalid_argument;
  std::stringstream buffer;
  buffer << fs.rdbuf();
  config_str = buffer.str();
  return reinforcement_learning::error_code::success;
}
