#include "example.h"
#include "factory_resolver.h"
#include "live_model.h"
#include <memory>
#include "config_utility.h"
#include <cassert>

// namespace manipulation for brevity
using namespace reinforcement_learning;
namespace cfg_util = reinforcement_learning::utility::config;
namespace m = model_management;
namespace u = utility;

char const * const LOCAL_FILE_TRANSPORT = "LOCAL_FILE";

// Forward declaration
class local_model_file;
int file_tranport_create(m::i_data_transport** , const u::config_collection& , api_status*);
std::unique_ptr<live_model> init_override();
void use_this_action(size_t choosen_action);

// Create and run RL interface
void override_transport_usage() {

  // Create an interface to reinforcement learning loop 
  // and initialize it
  auto rl = init_override();
  //  MODEL_SRC

  // Response class
  ranking_response response;

  // Choose an action
  auto scode = rl->choose_rank(uuid, context, response);
  assert(scode == error_code::success);
  size_t choosen_action;
  scode = response.get_choosen_action_id(choosen_action);
  assert(scode == error_code::success);

  // Use the response
  use_this_action(choosen_action);

  // Report reward recieved
  scode = rl->report_outcome(uuid, reward);
  assert(scode == error_code::success);
}

std::unique_ptr<live_model> init_override() {
  // Register local file transport in data_transport factory
  data_transport_factory.register_type(LOCAL_FILE_TRANSPORT, file_tranport_create);

  //create a configuration object from json data
  const auto config = cfg_util::init_from_json(JSON_CFG_DATA);

  //create the rl live_model, and initialize it with the config
  auto rl = std::make_unique<live_model>(config);
  rl->init(nullptr);

  return rl;
}

int file_tranport_create(m::i_data_transport** retval, const u::config_collection& cfg, api_status* status) {
  return error_code::success;
}

class local_model_file : public m::i_data_transport {

public:
  int get_data(model_management::model_data& data, api_status* status) override {
    data = read_new_data_file();
    return error_code::success;
  }

private:
  m::model_data read_new_data_file() {
    m::model_data md;
    // read this data from a local file if the data did not change
    md.data = new char[1000];
    md.data_sz = 1000;
    return md;
  }
};
