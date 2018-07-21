#pragma once
#include "object_factory.h"

namespace reinforcement_learning  {

  // Forward declarations
  namespace model_management {
    class i_data_transport;
    class i_model;
  }

  using dtfactory = utility::object_factory<model_management::i_data_transport>;
  using modelfactory = utility::object_factory<model_management::i_model>;

  extern dtfactory& data_transport_factory;
  extern modelfactory& model_factory;

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
