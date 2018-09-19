#pragma once
#include <string>

#define TRACE_LOG( logger, msg ) do{  \
    if(logger != nullptr) {           \
      logger->log(msg);               \
    }                                 \
  } while(0)                          \

namespace reinforcement_learning {
  class i_trace {
  public:
    virtual void log(const std::string& msg) = 0;
    virtual ~i_trace() {} ;
  };
}
