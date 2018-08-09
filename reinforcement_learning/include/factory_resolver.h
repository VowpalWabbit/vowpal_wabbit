#pragma once
#include "object_factory.h"

namespace reinforcement_learning  {
  namespace utility {
    class config_collection;
  }

  // Forward declarations
  namespace model_management {
    class i_data_transport;
    class i_model;
  }
  class i_logger;
  class error_callback_fn;

  /**
   * @brief Factory to create transport for model data.
   * Advanced extension point:  Register another implementation of i_data_transport to
   * provide updated model data used to hydrate inference model.
   */
  using data_transport_factory_t = utility::object_factory<model_management::i_data_transport, const utility::config_collection&>;
  /**
   * @brief Factory to create model used in inference.
   * Advanced extension point:  Register another implementation of i_model to
   * provide hydraded model given updated model data. This model is then used
   * in inference.
   */
  using model_factory_t = utility::object_factory<model_management::i_model, const utility::config_collection&>;
  using logger_factory_t = utility::object_factory<i_logger, const utility::config_collection&, error_callback_fn*>;

  extern data_transport_factory_t& data_transport_factory;
  extern model_factory_t& model_factory;
  extern logger_factory_t& logger_factory;

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
