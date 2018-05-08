#pragma once
#include "logger/ds_logger.h"

namespace decision_service
{
  class ranking_response;
  class api_status;

  class driver_impl {
  public:
    using error_fn = void(*)( const api_status&, void* user_context );

  private:
    utility::config_collection _configuration;
    error_callback_fn _error_cb;
    logger _logger;

  public:
    
    int ranking_request(const char* uuid, const char* context, ranking_response& response, api_status* status);
    //here the uuid is auto-generated
    int ranking_request(const char* context, ranking_response& response, api_status* status);
    
    int report_outcome(const char* uuid, const char* outcome_data, api_status* status);
    int report_outcome(const char* uuid, float reward, api_status* status);
    
    explicit driver_impl(const utility::config_collection& config, error_fn fn = nullptr, void* err_context = nullptr);
  };
}
