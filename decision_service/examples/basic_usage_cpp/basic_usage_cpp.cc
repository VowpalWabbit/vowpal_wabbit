// Reinforcement learning API demo
#include "basic_usage_cpp.h"

int api_example() {
  u::config_collection config;
  auto scode = load_config_from_json("client.json", config);
  RETURN_ON_ERROR_STR(scode, "Unable to Load file: client.json");

  // api_status is an optional argument used to return status of all API calls
  r::api_status status;

  // Create an interface to reinforcement learning loop using config
  auto rl = std::make_unique<r::live_model>(config);
  scode = rl->init(&status);
  RETURN_ON_ERROR(scode, status);

  // Response class
  r::ranking_response response;

  // Choose an action
  scode = rl->choose_rank(uuid, context, response, &status);
  RETURN_ON_ERROR(scode, status);

  size_t choosen_action;
  scode = response.get_choosen_action_id(choosen_action);
  RETURN_ON_ERROR(scode, status);

  // Use the response
  std::cout << "Chosen action id is: " << choosen_action << std::endl;

  // Report reward recieved
  scode = rl->report_outcome(uuid, reward, &status);
  RETURN_ON_ERROR(scode, status);

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
  return cfg::create_from_json(config_str);
}

// Load contents of file into a string
std::string load_file(const std::string& file_name) {
  std::ifstream fs;
  fs.open(file_name);
  if ( !fs.good() )
    throw std::runtime_error(u::concat("Unable to open file. ", file_name));
  std::stringstream buffer;
  buffer << fs.rdbuf();
  return buffer.str();
}
