#pragma once
#include "trace_logger.h"
namespace reinforcement_learning {
  class console_tracer : public i_trace {
    // Inherited via i_trace
     void log(const std::string & msg) override;
  };
}
