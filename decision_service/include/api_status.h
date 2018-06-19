#pragma once

#include <string>
#include <sstream>
#include "err_constants.h"

namespace reinforcement_learning {
  class api_status
  {
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

  template <typename Last>
  void report_error(std::ostringstream& os, const Last& last) {
    os << last;
  }

  template <typename First, typename ... Rest>
  void report_error(std::ostringstream& os, const First& first, const Rest& ... rest) {
    os << first;
    report_error(os, rest...);
  }

  template <typename ... All>
  int report_error(api_status* status, int scode, const All& ... all) {
    if ( status != nullptr ) {
      std::ostringstream os;
      report_error(os, all...);
      api_status::try_update(status, scode, os.str().c_str());
    }
    return scode;
  }

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
    sb._os << val;
  }
  return sb;
}

#define RETURN_STATUS(status, code)                         \
reinforcement_learning::status_builder sb(status, reinforcement_learning::error_code::code);    \
return sb << reinforcement_learning::error_code::code ## _s \

// this macro assumes that success_code equals 0
#define TRY_OR_RETURN(x) do {   \
  int retval__LINE__ = (x);     \
  if (retval__LINE__ != 0) {    \
    return retval__LINE__;      \
  }                             \
} while (0)                     \

#define RETURN_ERROR(status, errcode, errstr) do {       \
  reinforcement_learning::api_status::try_update(status, errcode, errstr);  \
  return errcode;                                   \
}while(0);                                          \

