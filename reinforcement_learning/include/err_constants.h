/**
 * @brief Definition of all API error return codes and descriptions 
 * 
 * @file err_constants.h
 * @author Rajan Chari et al
 * @date 2018-07-18
 */
#pragma once

namespace reinforcement_learning { namespace error_code {
  //! [Error Codes]
  //success code
  const int success = 0;

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
  const int model_update_error          = 24;
  const int model_rank_error            = 25;
  const int pdf_sampling_error          = 26;
  const int eh_connstr_parse_error      = 27;
  //! [Error Codes]
}}

namespace reinforcement_learning { namespace error_code {
  char const * const unkown_s                   = "Unexpected error.";
  //! [Error Description]
  char const * const create_fn_exception_s      = "Create function failed.";
  char const * const type_not_registered_s      = "Type not registered with class factory";
  char const * const http_uri_not_provided_s    = "URL parameter was not passed in via configuration";
  char const * const http_bad_status_code_s     = "http request returned a bad status code";
  char const * const last_modified_not_found_s  = "Last-Modified http header not found in response";
  char const * const last_modified_invalid_s    = "Unable to parse Last-Modified http header as date-time";
  char const * const bad_content_length_s       = "Content-Length header not set or set to zero";
  char const * const model_export_frequency_not_provided_s = "Export frequency of model not specified in configuration.";
  char const * const bad_time_interval_s        = "Bad time interval string.  Format should be hh:mm:ss";
  char const * const data_callback_exception_s  = "Background data callback threw an exception. ";
  char const * const data_callback_not_set_s    = "Data callback handler not set";
  char const * const json_no_actions_found_s    = "Context json did not have actions (_multi array empty or not found)";
  char const * const exploration_error_s        = "Exploration error code: ";
  char const * const model_rank_error_s         = "Error while ranking actions using model: ";
  char const * const model_update_error_s       = "Error updating model: ";
  char const * const action_not_found_s         = "No actions found in action collection";
  char const * const action_out_of_bounds_s     = "Action id out of bounds.";
  char const * const background_thread_start_s  = "Unable to start background thread. ";
  char const * const background_queue_overflow_s = "Background queue overflow. ";
  char const * const eventhub_http_generic_s    = "http error while connecting to event hub. ";
  char const * const json_parse_error_s         = "Unable to parse JSON. ";
  char const * const invalid_argument_s         = "Invalid Argument: ";
  char const * const exception_during_http_req_s = "http request excepton. ";
  char const * const eh_connstr_parse_error_s = "Unable to parse event hub connection connection string.";
  //! [Error Description]
}}
