#pragma once
#include <boost/program_options.hpp>
#include "error_context.h"
#include "person.h"
#include "live_model.h"

class rl_sim {
public:
  explicit rl_sim(boost::program_options::variables_map vm);
  std::string create_context_json(const std::string& cntxt, const std::string& action);
  std::string create_uuid();
  int loop();

  private:
    person& pick_a_random_person();
    int load_config_from_json(
      const std::string& str, 
      reinforcement_learning::utility::config_collection& config,
      reinforcement_learning::api_status* status);
    int load_file(const std::string& file_name, std::string& config_str);
    int init_rl();
    bool init_people();
    bool init();
    std::string get_action_features();
  
  private:
    boost::program_options::variables_map _options;
    std::unique_ptr<reinforcement_learning::live_model> _rl;
    std::vector<person> _people;
    std::vector<std::string> _actions;
    error_context _error_context;
};

