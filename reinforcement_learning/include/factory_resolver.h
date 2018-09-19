#pragma once
#include "object_factory.h"
#include  "trace_logger.h"

namespace reinforcement_learning  {
  namespace utility {
    class configuration;
    class watchdog;
  }

  // Forward declarations
  namespace model_management {
    class i_data_transport;
    class i_model;
  }
  class i_logger;
  class i_trace;
  class error_callback_fn;

  /**
  * @brief Factory to create trace loggers used to trace events in the library.
  * Advanced extension point:  Register another implementation of i_trace to
  * provide the mechanism used when logging internal events in the API implementation.
  */
  using trace_logger_factory_t = utility::object_factory<i_trace, const utility::configuration&>;
  /**
  * @brief Factory to create model used in inference.
  * Advanced extension point:  Register another implementation of i_model to
  * provide hydraded model given updated model data. This model is then used
  * in inference.
  */
  using model_factory_t = utility::object_factory<model_management::i_model, const utility::configuration&>;
  /**
   * @brief Factory to create transport for model data.
   * Advanced extension point:  Register another implementation of i_data_transport to
   * provide updated model data used to hydrate inference model.
   */
  using data_transport_factory_t = utility::object_factory<model_management::i_data_transport, const utility::configuration&>;
  /**
   * @brief Factory to create loggers used to record the interactions and observations.
   * Advanced extension point:  Register another implementation of i_logger to
   * provide the mechanism used when logging interaction and observation events.
   */
  using logger_factory_t = utility::object_factory<i_logger, const utility::configuration&, utility::watchdog&, error_callback_fn*>;

  extern data_transport_factory_t& data_transport_factory;
  extern model_factory_t& model_factory;
  extern logger_factory_t& logger_factory;
  extern trace_logger_factory_t& trace_logger_factory;

  // For proper static intialization
  // Check https://en.wikibooks.org/wiki/More_C++_Idioms/Nifty_Counter for explanation
  struct factory_initializer {
    factory_initializer();
    ~factory_initializer();
    private:
    static void register_default_factories();
  };
  // Every translation unit gets a factory_initializer
  // only one translation unit will initialize it
  static factory_initializer _init;
}
