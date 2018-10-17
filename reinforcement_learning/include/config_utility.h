#pragma once
#include <string>

namespace reinforcement_learning {namespace utility {
  class configuration;
}
class api_status;
class i_trace;
}

namespace reinforcement_learning { namespace utility { namespace config {
  std::string load_config_json();
  int create_from_json(const std::string& config_json, configuration& cc, i_trace* trace = nullptr, api_status* = nullptr);
}}}
