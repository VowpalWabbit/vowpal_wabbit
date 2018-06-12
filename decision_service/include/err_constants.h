#pragma once

namespace reinforcement_learning { namespace error_code {
  //success code
  const int success = 0;

  //error code
  const int invalid_argument            = 1;
  const int background_queue_overflow   = 2;
  const int eventhub_http_generic       = 3;
  const int http_bad_status_code        = 4;
  const int action_not_found            = 5;
  const int background_thread_start     = 6;
  const int not_initialized             = 7;
  const int eventhub_generate_SAS_hash  = 8;
  const int create_fn_exception         = 9;
  const int type_not_registered         = 10;
  const int http_uri_not_provided       = 11;
  const int last_modified_not_found     = 12;
  const int last_modified_invalid       = 13;
  const int bad_content_length          = 14;
  const int exception_during_http_req   = 15;
  const int model_export_frequency_not_provided = 16;
  const int bad_time_interval           = 17;
  const int data_callback_exception     = 18;
  const int data_callback_not_set       = 19;
  const int json_no_actions_found       = 20;
  const int json_parse_error            = 21;
  const int exploration_error           = 22;
  const int action_out_of_bounds        = 23;
}}

namespace reinforcement_learning { namespace error_code {
  using errstr_t = char const * const;
  //error message
  errstr_t unkown_s                   = "Unexpected error.";
  errstr_t create_fn_exception_s      = "Create function failed.";
  errstr_t type_not_registered_s      = "Type not registered with class factory";
  errstr_t http_uri_not_provided_s    = "URL parameter was not passed in via config_collection";
  errstr_t http_bad_status_code_s     = "http request returned a bad status code";
  errstr_t last_modified_not_found_s  = "Last-Modified http header not found in response";
  errstr_t last_modified_invalid_s    = "Unable to parse Last-Modified http header as date-time";
  errstr_t bad_content_length_s       = "Content-Length header not set or set to zero";
  errstr_t model_export_frequency_not_provided_s = "Export frequency of model not specified in configuration.";
  errstr_t bad_time_interval_s        = "Bad time interval string.  Format should be hh:mm:ss";
  errstr_t data_callback_exception_s  = "Background data callback threw an exception. ";
  errstr_t data_callback_not_set_s    = "Data callback handler not set";
  errstr_t json_no_actions_found_s    = "Context json did not have actions (_multi array empty or not found)";
}}
