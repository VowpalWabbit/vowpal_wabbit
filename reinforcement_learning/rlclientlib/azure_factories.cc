#include "factory_resolver.h"
#include "constants.h"
#include "model_mgmt/restapi_data_transport.h"
#include "logger/event_logger.h"

namespace reinforcement_learning {
  namespace m = model_management;
  namespace u = utility;

  int restapi_data_tranport_create(m::i_data_transport** retval, const u::configuration& config, i_trace* trace_logger, api_status* status);
  int vw_model_create(m::i_model** retval, const u::configuration&, i_trace* trace_logger, api_status* status);
  int observation_sender_create(i_sender** retval, const u::configuration&, error_callback_fn*, i_trace* trace_logger, api_status* status);
  int interaction_sender_create(i_sender** retval, const u::configuration&, error_callback_fn*, i_trace* trace_logger, api_status* status);
  int null_tracer_create(i_trace** retval, const u::configuration&, i_trace* trace_logger, api_status* status);
  int console_tracer_create(i_trace** retval, const u::configuration&, i_trace* trace_logger, api_status* status);

  void register_azure_factories() {
    data_transport_factory.register_type(value::AZURE_STORAGE_BLOB, restapi_data_tranport_create);
    sender_factory.register_type(value::OBSERVATION_EH_SENDER, observation_sender_create);
    sender_factory.register_type(value::INTERACTION_EH_SENDER, interaction_sender_create);
  }

  int restapi_data_tranport_create(m::i_data_transport** retval, const u::configuration& config, i_trace* trace_logger, api_status* status) {
    const auto uri = config.get(name::MODEL_BLOB_URI, nullptr);
    if (uri == nullptr) {
      api_status::try_update(status, error_code::http_uri_not_provided, error_code::http_uri_not_provided_s);
      return error_code::http_uri_not_provided;
    }
    auto pret = new m::restapi_data_tranport(uri, trace_logger);
    const auto scode = pret->check(status);
    if (scode != error_code::success) {
      delete pret;
      return scode;
    }
    *retval = pret;
    return error_code::success;
  }

  int observation_sender_create(i_sender** retval, const u::configuration& cfg, error_callback_fn* _error_cb, i_trace* trace_logger, api_status* status) {
    *retval = new eventhub_client(
      cfg.get(name::OBSERVATION_EH_HOST, "localhost:8080"),
      cfg.get(name::OBSERVATION_EH_KEY_NAME, ""),
      cfg.get(name::OBSERVATION_EH_KEY, ""),
      cfg.get(name::OBSERVATION_EH_NAME, "observation"),
      cfg.get_int(name::OBSERVATION_EH_TASKS_LIMIT, 16),
      trace_logger,
      _error_cb,
      cfg.get_bool(name::EH_TEST, false));
    return error_code::success;
  }

  int interaction_sender_create(i_sender** retval, const u::configuration& cfg, error_callback_fn* _error_cb, i_trace* trace_logger, api_status* status) {
    *retval = new eventhub_client(
      cfg.get(name::INTERACTION_EH_HOST, "localhost:8080"),
      cfg.get(name::INTERACTION_EH_KEY_NAME, ""),
      cfg.get(name::INTERACTION_EH_KEY, ""),
      cfg.get(name::INTERACTION_EH_NAME, "interaction"),
      cfg.get_int(name::INTERACTION_EH_TASKS_LIMIT, 16),
      trace_logger,
      _error_cb,
      cfg.get_bool(name::EH_TEST, false));
    return error_code::success;
  }
}
