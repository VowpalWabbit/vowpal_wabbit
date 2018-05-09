#pragma once

#include <string>


namespace decision_service { namespace error_code {
    //success code
    const int success = 0;

    //error code
    const int invalid_argument = 1;
    const int background_queue_overflow = 2;
    const int eventhub_http_generic = 3;
    const int eventhub_http_bad_status_code = 4;
}}

namespace decision_service {
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
