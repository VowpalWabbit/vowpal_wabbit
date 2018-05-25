#pragma once
#include "logger/logger.h"

namespace reinforcement_learning
{
  class ranking_response;
  class api_status;

  class live_model_impl {
  public:
    using error_fn = void(*)( const api_status&, void* user_context );

  private:
    utility::config_collection _configuration;
    error_callback_fn _error_cb;
    logger _logger;
    std::ostringstream _buff;

  public:
    int init(api_status* status);

    int choose_rank(const char* uuid, const char* context, ranking_response& response, api_status* status);
    //here the uuid is auto-generated
    int choose_rank(const char* context, ranking_response& response, api_status* status);
    
    int report_outcome(const char* uuid, const char* outcome_data, api_status* status);
    int report_outcome(const char* uuid, float reward, api_status* status);
    
    explicit live_model_impl(const utility::config_collection& config, error_fn fn = nullptr, void* err_context = nullptr);
  };
}
