#include <cstring>
#include "logger/async_batcher.h"

namespace reinforcement_learning {
    queue_mode_enum to_queue_mode_enum(const char* queue_mode) {
    if (std::strcmp(queue_mode, "BLOCK") == 0) {
      return BLOCK;
    } else {
      return DROP;
    }
  }
};
