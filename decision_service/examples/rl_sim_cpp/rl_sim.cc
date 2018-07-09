#include <iostream>
#include <fstream>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "rl_sim.h"
#include "ranking_response.h"
#include "person.h"
#include "live_model.h"
#include "rl_sim_cpp.h"
#include "config_utility.h"

namespace r = reinforcement_learning;
namespace u = reinforcement_learning::utility;
namespace c = reinforcement_learning::utility::config;

rl_sim::rl_sim(boost::program_options::variables_map vm):_options(std::move(vm)) {}
std::string rl_sim::create_context_json(const std::string& cntxt, const std::string& action) {
  std::ostringstream oss;
  oss << "{ " << cntxt << ", " << action << " }";
  return oss.str();
}

std::string rl_sim::create_uuid() {
  return boost::uuids::to_string(boost::uuids::random_generator()( ));
}

int rl_sim::loop() {
  if ( !init() ) return r::error_code::success;
  
  auto round = 0;
  r::ranking_response response;
  while(true) {
    auto& p = pick_a_random_person();
    const auto context_features = p.get_features();
    const auto action_features = get_action_features();
    const auto context_json = create_context_json(context_features,action_features);
    const auto req_id = create_uuid();  
    r::api_status status;

    // Choose an action
    RETURN_ON_ERROR(_rl->choose_rank(req_id.c_str(), context_json.c_str(), response, &status), status);
    size_t choosen_action;
    RETURN_ON_ERROR(response.get_choosen_action_id(choosen_action), status);

    // What reward did this action get?
    const auto reward = p.get_reward(_actions[choosen_action]);

    // Report reward recieved
    RETURN_ON_ERROR(_rl->report_outcome(req_id.c_str(), reward, &status), status);

    std::cout << "Round:" << round << ", Person:" << p.id() << ", Action:" << choosen_action << ", Reward:" << reward << std::endl;
    response.clear();
    ++round;
  }
}

person& rl_sim::pick_a_random_person() {
  return _people[rand() % _people.size()];
}

int rl_sim::load_config_from_json(  const std::string& file_name, 
                                    u::config_collection& cfgcoll,
                                    r::api_status* status) {
  std::string config_str;

  // Load contents of config file into a string
  RETURN_IF_FAIL(load_file(file_name, config_str));

  // Use library supplied convinence method to parse json and build config object
  return c::create_from_json(config_str, cfgcoll, status);
}

int rl_sim::load_file(const std::string& file_name, std::string& config_str) {
  std::ifstream fs;
  fs.open(file_name);
  if ( !fs.good() )
    return reinforcement_learning::error_code::invalid_argument;
  std::stringstream buffer;
  buffer << fs.rdbuf();
  config_str = buffer.str();
  return reinforcement_learning::error_code::success;
}

int rl_sim::init_rl() {
  r::api_status status;
  u::config_collection config;

  const auto cfg_file = _options["json_config"].as<std::string>();
  RETURN_ON_ERROR(load_config_from_json(cfg_file, config, &status), status);

  _rl = std::unique_ptr<r::live_model>(new r::live_model(config));
  RETURN_ON_ERROR(_rl->init(&status), status);

  return r::error_code::success;
}

bool rl_sim::init_people() {

  person::topic_prob tp1 { 
    { "HerbGarden",0.002f }, 
    { "MachineLearning",0.03 } };
  _people.emplace_back("rnc", "engineering", "hiking", "spock", tp1);

  person::topic_prob tp2 {
    { "HerbGarden",0.015f },
    { "MachineLearning",0.05 } };
  _people.emplace_back("mk", "psychology", "kids", "7of9", tp2);

  _actions.emplace_back("HerbGarden");
  _actions.emplace_back("MachineLearning");

  return true;
}

bool rl_sim::init() {
  if ( init_rl() != r::error_code::success ) return false;
  if ( !init_people() ) return false;
  return true;
}

std::string rl_sim::get_action_features() {
  std::ostringstream oss;
  oss << R"("_multi": [ {"topic":"HerbGarden"}, {"topic":"MachineLearning"} ])";
  return oss.str();
}

