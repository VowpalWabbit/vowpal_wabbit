#include "trace_logger.h"

const char* get_log_level_string(int log_level) {
  switch ( log_level ) {
  case reinforcement_learning::LEVEL_DEBUG:
    return reinforcement_learning::STR_LEVEL_DEBUG;
  case reinforcement_learning::LEVEL_INFO:
    return reinforcement_learning::STR_LEVEL_INFO;
  case reinforcement_learning::LEVEL_WARN:
    return reinforcement_learning::STR_LEVEL_WARN;
  case reinforcement_learning::LEVEL_ERROR:
    return reinforcement_learning::STR_LEVEL_ERROR;
  default:
    return "LOG";
  }
}
