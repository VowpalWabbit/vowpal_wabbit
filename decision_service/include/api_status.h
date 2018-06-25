#pragma once

#include <string>
#include <sstream>
#include "err_constants.h"

namespace reinforcement_learning {
  class api_status {
  public:
    api_status();

    int get_error_code() const;
    const char* get_error_msg() const;

    static void try_update(api_status* status, int new_code, const char* new_msg);
    static void try_clear(api_status* status);

  private:
    int _error_code;
    std::string _error_msg;
  };

  struct status_builder {
    status_builder(api_status* status, int code);
    ~status_builder();
    operator int() const;

    int _code;
    api_status* _status;
    std::ostringstream _os;

    status_builder(const status_builder&&) = delete;
    status_builder(const status_builder&) = delete;
    status_builder& operator=(const status_builder&) = delete;
    status_builder& operator=(status_builder&&) = delete;
  };
}

template <typename T>
reinforcement_learning::status_builder& operator <<(reinforcement_learning::status_builder& sb, const T& val) {
  if ( sb._status != nullptr ) {
    sb._os << ", " << val;
  }
  return sb;
}

namespace reinforcement_learning {
  inline int report_error(status_builder& sb) {
    return sb;
  }
    
  template <typename Last>
  int report_error(status_builder& sb, const Last& last) {
    return sb << last;
  }

  template <typename First, typename ... Rest>
  int report_error(status_builder& sb, const First& first, const Rest& ... rest) {
    sb << first;
    return report_error(sb, rest...);
  }
}

#define RETURN_ERROR_ARG(status, code, ... ) do {                                                 \
  if(status != nullptr) {                                                                         \
    reinforcement_learning::status_builder sb(status, reinforcement_learning::error_code::code);  \
    sb << reinforcement_learning::error_code::code ## _s;                                         \
    return report_error(sb, __VA_ARGS__ );                                                        \
  }                                                                                               \
  return reinforcement_learning::error_code::code;                                                                                    \
} while(0);                                                                                       \

#define RETURN_ERROR_LS(status, code)                                                             \
reinforcement_learning::status_builder sb(status, reinforcement_learning::error_code::code);      \
return sb << reinforcement_learning::error_code::code ## _s                                       \

// this macro assumes that success_code equals 0
#define RETURN_IF_FAIL(x) do {   \
  int retval__LINE__ = (x);     \
  if (retval__LINE__ != 0) {    \
    return retval__LINE__;      \
  }                             \
} while (0)                     \

