#include "trace_logger.h"

const char* get_log_level_string(int log_level) {
  switch ( log_level ) {
  case online_trainer::LEVEL_DEBUG:
    return online_trainer::STR_LEVEL_DEBUG;
  case online_trainer::LEVEL_INFO:
    return online_trainer::STR_LEVEL_INFO;
  case online_trainer::LEVEL_WARN:
    return online_trainer::STR_LEVEL_WARN;
  case online_trainer::LEVEL_ERROR:
    return online_trainer::STR_LEVEL_ERROR;
  default:
    return "LOG";
  }
}
