#pragma once

namespace reinforcement_learning { namespace error_code {
  //success code
  const int success = 0;

  //error code
  const int invalid_argument            = 1;
  const int background_queue_overflow   = 2;
  const int eventhub_http_generic       = 3;
  const int eventhub_http_bad_status_code = 4;
  const int action_not_found            = 5;
  const int background_thread_start     = 6;
  const int not_initialized             = 7;
  const int eventhub_generate_SAS_hash  = 8;
  const int create_fn_exception         = 9;
  const int type_not_registered         = 10;
  const int uri_not_provided            = 11;
}}

namespace reinforcement_learning { namespace error_code {
  using errstr_t = char const * const;
  //error message
  errstr_t create_fn_exception_s = "Create function failed.";
  errstr_t type_not_registered_s = "Type not registered with class factory";
  errstr_t uri_not_provided_s = "URL parameter was not passed in via config_collection";
}}
