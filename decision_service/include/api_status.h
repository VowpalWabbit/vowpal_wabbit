#pragma once

#include <string>
#include <sstream>

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

  template <typename ... All>
  int report_error(api_status* status, int scode, const All& ... all) {
    if ( status != nullptr ) {
      std::ostringstream os;
      report_error(os, all...);
      api_status::try_update(status, scode, os.str().c_str());
    }
    return scode;
  }

  template <typename First, typename ... Rest>
  void report_error(std::ostream& os, const First& first, const Rest& ... rest) {
    os << first;
    report_error(os, rest...);
  }

  template <typename Last>
  void report_error(std::ostream& os, const Last& last) {
    os << last;
  }

}

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

