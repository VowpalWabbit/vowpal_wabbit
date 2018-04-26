#include "ds_config_collection.h"

namespace decision_service { namespace utility {
  void config_collection::set(const char* name, const char* value) {
    _map[name] = value;
  }
}
}
