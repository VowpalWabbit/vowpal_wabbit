#include "../include/ds_utility.h"

namespace decision_service { namespace utility { namespace config {
  std::string load_config_json()
  { //TODO: Load appid configuration from Azure storage
    //TODO: error handling.  (return code or exception)

    return "";
  }

  config_collection init_from_json(const std::string& config_json)
  { //TODO: Load configuration from json string
    //TODO: error handling.  (return code or exception)

    config_collection cc;
    cc.set("name", "value");
    return cc;
  }

}}}
