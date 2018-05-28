#pragma once
#include "model_mgmt.h"
#include "object_factory.h"

namespace reinforcement_learning  {
  using dtfactory = utility::object_factory<model_mangement::i_data_transport>;
  using modelfactory = utility::object_factory<model_mangement::i_model>;

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
