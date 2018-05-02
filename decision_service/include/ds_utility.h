#pragma once
#include <string>
#include "../dsclientlib/ds_config_collection.h"

namespace decision_service { namespace utility { namespace config {
  std::string load_config_json();
  config_collection init_from_json(const std::string& config_json);
}}}
