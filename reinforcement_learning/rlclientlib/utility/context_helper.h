#pragma once
#include "api_status.h"

namespace reinforcement_learning {
  class i_trace;
  namespace utility {
  int get_action_count(size_t& count, const char *context, i_trace* trace, api_status* status = nullptr);
}}
