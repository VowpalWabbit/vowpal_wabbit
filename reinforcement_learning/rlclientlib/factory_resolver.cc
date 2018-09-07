#include "factory_resolver.h"

#include "constants.h"
#include "err_constants.h"
#include "model_mgmt/restapi_data_transport.h"
#include "vw_model/vw_model.h"
#include "logger/event_hub_logger.h"
#include "utility/watchdog.h"

#include <type_traits>

namespace reinforcement_learning {
  namespace m = model_management;
  namespace u = utility;
  // For proper static intialization
  // Check https://en.wikibooks.org/wiki/More_C++_Idioms/Nifty_Counter for explanation
  static int init_guard;  // guaranteed to be zero when loaded

  // properly aligned memory for the factory object
  template <typename T>
  using natural_align = std::aligned_storage<sizeof(T), alignof ( T )>;

  static natural_align<data_transport_factory_t>::type dtfactory_buf;
  static natural_align<model_factory_t>::type modelfactory_buf;
  static natural_align<logger_factory_t>::type loggerfactory_buf;

  // Reference should point to the allocated memory to be initialized by placement new in factory_initializer::factory_initializer()
  data_transport_factory_t& data_transport_factory = (data_transport_factory_t&)( dtfactory_buf );
  model_factory_t& model_factory = (model_factory_t&)( modelfactory_buf );
  logger_factory_t& logger_factory = (logger_factory_t&)( loggerfactory_buf );

  factory_initializer::factory_initializer() {
    if ( init_guard++ == 0 ) {
      new ( &data_transport_factory ) data_transport_factory_t();
      new ( &model_factory ) model_factory_t();
      new ( &logger_factory ) logger_factory_t();

      register_default_factories();
    }
  }

  factory_initializer::~factory_initializer() {
    if ( --init_guard == 0 ) {
      ( &data_transport_factory )->~data_transport_factory_t();
      ( &model_factory )->~model_factory_t();
      ( &logger_factory )->~logger_factory_t();
    }
  }

  int restapi_data_tranport_create(m::i_data_transport** retval, const u::configuration& config, api_status* status);
  int vw_model_create(m::i_model** retval, const u::configuration&, api_status* status);
  int observation_logger_create(i_logger** retval, const u::configuration&, u::watchdog& watchdog, error_callback_fn*,  api_status* status);
  int interaction_logger_create(i_logger** retval, const u::configuration&, u::watchdog& watchdog, error_callback_fn*, api_status* status);

  void factory_initializer::register_default_factories() {
    data_transport_factory.register_type(value::AZURE_STORAGE_BLOB, restapi_data_tranport_create);
    model_factory.register_type(value::VW, vw_model_create);
    logger_factory.register_type(value::OBSERVATION_EH_LOGGER, observation_logger_create);
    logger_factory.register_type(value::INTERACTION_EH_LOGGER, interaction_logger_create);
  }

  int restapi_data_tranport_create(m::i_data_transport** retval, const u::configuration& config, api_status* status) {
    const auto uri = config.get(name::MODEL_BLOB_URI, nullptr);
    if ( uri == nullptr ) {
      api_status::try_update(status, error_code::http_uri_not_provided, error_code::http_uri_not_provided_s);
      return error_code::http_uri_not_provided;
    }
    auto pret = new m::restapi_data_tranport(uri);
    const auto scode = pret->check(status);
    if(scode != error_code::success) {
      delete pret;
      return scode;
    }
    *retval = pret;
    return error_code::success;
  }

  int vw_model_create(m::i_model** retval, const u::configuration&, api_status* status) {
    *retval = new m::vw_model();
    return error_code::success;
  }

  int observation_logger_create(i_logger** retval, const u::configuration& cfg, u::watchdog& watchdog, error_callback_fn* error_callback, api_status* status) {
    *retval = new event_hub_observation_logger(cfg, watchdog, error_callback);
    return error_code::success;
  }

  int interaction_logger_create(i_logger** retval, const u::configuration& cfg, u::watchdog& watchdog, error_callback_fn* error_callback, api_status* status) {
    *retval = new event_hub_interaction_logger(cfg, watchdog, error_callback);
    return error_code::success;
  }
}
