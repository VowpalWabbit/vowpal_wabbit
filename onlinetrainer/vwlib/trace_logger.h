#pragma once
#include <string>

namespace vw_lib {
  const int LEVEL_DEBUG = -10;
  const int LEVEL_INFO = 0;
  const int LEVEL_WARN = 10;
  const int LEVEL_ERROR = 20;

  const char *const STR_LEVEL_DEBUG = "DEBUG";
  const char *const STR_LEVEL_INFO = "INFO";
  const char *const STR_LEVEL_WARN = "WARN";
  const char *const STR_LEVEL_ERROR = "ERROR";
}

const char* get_log_level_string(int log_level);

#define TRACE_LOG( logger, level, msg ) do{  \
    if(logger != nullptr) {                  \
      logger->log(level, msg);               \
    }                                        \
  } while(0)                                 \

#define TRACE_DEBUG( logger, msg ) TRACE_LOG(logger, vw_lib::LEVEL_DEBUG, msg)
#define TRACE_INFO( logger, msg )  TRACE_LOG(logger, vw_lib::LEVEL_INFO, msg)
#define TRACE_WARN( logger, msg )  TRACE_LOG(logger, vw_lib::LEVEL_WARN, msg)
#define TRACE_ERROR( logger, msg ) TRACE_LOG(logger, vw_lib::LEVEL_ERROR, msg)

namespace vw_lib {
  class i_trace {
  public:
    virtual void log(int log_level, const std::string& msg) = 0;
    virtual ~i_trace() {};
  };
}
