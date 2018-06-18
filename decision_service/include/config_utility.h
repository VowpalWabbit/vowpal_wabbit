#pragma once
#include <string>
#include "config_collection.h"

namespace reinforcement_learning { namespace utility { namespace config {
  std::string load_config_json();
  config_collection create_from_json(const std::string& config_json);
}}}
