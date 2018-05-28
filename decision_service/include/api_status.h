#pragma once

#include <string>



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
}

// this macro assumes that success_code equals 0
#define TRY_OR_RETURN(x) do { \
  int retval__LINE__ = (x); \
  if (retval__LINE__ != 0) { \
    return retval__LINE__; \
  } \
} while (0)

