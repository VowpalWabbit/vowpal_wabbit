#pragma once
#include <string>

namespace reinforcement_learning {namespace utility {
  class configuration;
}
class api_status;
}

namespace reinforcement_learning { namespace utility { namespace config {
  std::string load_config_json();
  int create_from_json(const std::string& config_json, configuration& cc, api_status* = nullptr);
}}}
