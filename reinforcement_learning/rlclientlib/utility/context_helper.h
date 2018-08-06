#pragma once
#include "api_status.h"

namespace reinforcement_learning { namespace utility {
  int get_action_count(size_t& count, const char *context, api_status* status = nullptr);
}}
