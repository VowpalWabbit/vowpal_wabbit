#pragma once
#include <memory>
#include "logger/logger.h"
#include "model_mgmt.h"
#include "data_callback_fn.h"
#include "model_downloader.h"
#include "utility/object_pool.h"
#include "periodic_background_proc.h"
#include "object_factory.h"

namespace reinforcement_learning
{
class safe_vw_factory;
class safe_vw;
class ranking_response;
  class api_status;

  class live_model_impl {
  public:
    using error_fn = void(*)( const api_status&, void* user_context );
    using transport_factory_t = utility::object_factory<model_management::i_data_transport>;
    using model_factory_t = utility::object_factory<model_management::i_model>;

    int init(api_status* status);

    int choose_rank(const char* uuid, const char* context, ranking_response& response, api_status* status);
    //here the uuid is auto-generated
    int choose_rank(const char* context, ranking_response& response, api_status* status);
    
    int report_outcome(const char* uuid, const char* outcome_data, api_status* status);
    int report_outcome(const char* uuid, float reward, api_status* status);
    
    explicit live_model_impl(
      const utility::config_collection& config, 
      error_fn fn,
      void* err_context, 
      transport_factory_t* t_factory,
      model_factory_t* m_factory
      );

    live_model_impl(const live_model_impl&) = delete;
    live_model_impl(live_model_impl&&) = delete;
    live_model_impl& operator=(const live_model_impl&) = delete;
    live_model_impl& operator=(live_model_impl&&) = delete;

  private:
    // Internal implementation methods
    int init_model(api_status* status);
    int init_model_mgmt(api_status* status);
    static void _handle_model_update(const model_management::model_data& data, live_model_impl* ctxt);
    void handle_model_update(const model_management::model_data& data);
    int explore_only(const char* uuid, const char* context, ranking_response& response, api_status* status) const;
    int explore_exploit(const char* uuid, const char* context, ranking_response& response, api_status* status) const;

  private:
    // Internal implementation state
    bool _model_data_received = false;
    float _initial_epsilon = 0.2f;
    utility::config_collection _configuration;
    error_callback_fn _error_cb;
    model_management::data_callback_fn _data_cb;
    std::ostringstream _buff;
    logger _logger;
    transport_factory_t* _t_factory;
    model_factory_t* _m_factory;
    std::unique_ptr<model_management::i_data_transport> _transport;
    std::unique_ptr<model_management::i_model> _model;
    std::unique_ptr<model_management::model_downloader> _model_download;
    utility::periodic_background_proc<model_management::model_downloader> _bg_model_proc;
  };
}
