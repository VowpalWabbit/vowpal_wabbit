#include "trace_logger.h"

const char* get_log_level_string(int log_level) {
  switch (log_level) {
  case vw_lib::LEVEL_DEBUG:
    return vw_lib::STR_LEVEL_DEBUG;
  case vw_lib::LEVEL_INFO:
    return vw_lib::STR_LEVEL_INFO;
  case vw_lib::LEVEL_WARN:
    return vw_lib::STR_LEVEL_WARN;
  case vw_lib::LEVEL_ERROR:
    return vw_lib::STR_LEVEL_ERROR;
  default:
    return "LOG";
  }
}
