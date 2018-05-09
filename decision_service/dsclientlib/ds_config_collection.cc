#include "ds_config_collection.h"

namespace decision_service { namespace utility {

  void config_collection::set(const char* name, const char* value) {
    _map[name] = value;
  }

  const char* config_collection::get(const char* name, const char* defval) const {
    const auto it = _map.find(name);
    if (it != _map.end()) 
      return it->second.c_str();
    return defval;
  }

  int config_collection::get_int(const char* name, int defval) const {
    const auto it = _map.find(name);
    if (it != _map.end())
      return atoi(it->second.c_str());
    return defval;
  }
}}
