#pragma once
#include "logger/logger.h"
#include "model_mgmt.h"
#include "data_callback_fn.h"
#include "bg_model_download.h"
#include <memory>
#include <boost/uuid/string_generator.hpp>

namespace reinforcement_learning
{
  class ranking_response;
  class api_status;

  class live_model_impl {
  public:
    using error_fn = void(*)( const api_status&, void* user_context );

    int init(api_status* status);

    int choose_rank(const char* uuid, const char* context, ranking_response& response, api_status* status);
    //here the uuid is auto-generated
    int choose_rank(const char* context, ranking_response& response, api_status* status);
    
    int report_outcome(const char* uuid, const char* outcome_data, api_status* status);
    int report_outcome(const char* uuid, float reward, api_status* status);
    
    explicit live_model_impl(const utility::config_collection& config, error_fn fn = nullptr, void* err_context = nullptr);

    live_model_impl(const live_model_impl&) = delete;
    live_model_impl(live_model_impl&&) = delete;
    live_model_impl& operator=(const live_model_impl&) = delete;
    live_model_impl& operator=(live_model_impl&&) = delete;

  private:
    // Implementation details
    int init_model(api_status* status);
    int init_model_mgmt(api_status* status);
    static void _handle_model_update(const model_management::model_data& data, live_model_impl* ctxt);
    void handle_model_update(const model_management::model_data& data);
    int explore_only(const char* uuid, const char* context, ranking_response& response, api_status* status) const;
    int explore_exploit(const char* uuid, const char* context, ranking_response& response, api_status* status);

  private:
    bool _model_data_received;
    utility::config_collection _configuration;
    error_callback_fn _error_cb;
    logger _logger;
    std::ostringstream _buff;
    std::unique_ptr<model_management::i_model> _model;
    std::unique_ptr<model_management::i_data_transport> _model_transport;
    std::unique_ptr<model_management::data_callback_fn> _data_cb;
    std::unique_ptr<model_management::bg_model_download> _bg_model_download;
    float _initial_epsilon;
    boost::uuids::string_generator _uuid;
  };
}
