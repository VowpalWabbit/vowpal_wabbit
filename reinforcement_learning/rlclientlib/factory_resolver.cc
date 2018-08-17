#include <type_traits>
#include "factory_resolver.h"
#include "constants.h"
#include "model_mgmt/restapi_data_transport.h"
#include "vw_model/vw_model.h"
#include "err_constants.h"

namespace reinforcement_learning { 
  namespace m = model_management;
  namespace u = utility;
  // For proper static intialization 
  // Check https://en.wikibooks.org/wiki/More_C++_Idioms/Nifty_Counter for explanation
  static int init_guard;  // guranteed to be zero when loaded

  // properly aligned memory for the factory object 
  template <typename T>
  using natural_align = std::aligned_storage<sizeof(T), alignof ( T )>;

  static natural_align<dtfactory>::type dtfactory_buf;
  static natural_align<modelfactory>::type modelfactory_buf;

  // reference should point to the allocated memory to be initalized by placement new in factory_initializer::factory_initializer()
  dtfactory& data_transport_factory = (dtfactory&)( dtfactory_buf );
  modelfactory& model_factory = (modelfactory&)( modelfactory_buf );

  factory_initializer::factory_initializer() {
    if ( init_guard++ == 0 ) {
      new ( &data_transport_factory ) dtfactory();
      new ( &model_factory ) modelfactory();

      register_default_factories();
    }
  }

  factory_initializer::~factory_initializer() {
    if ( --init_guard == 0 ) {
      ( &data_transport_factory )->~dtfactory();
      ( &model_factory )->~modelfactory();
    }
  }

  int restapi_data_tranport_create(m::i_data_transport** retval, const u::configuration& cfg, api_status* status);
  int vw_model_create(m::i_model** retval, const u::configuration&, api_status* status);

  void factory_initializer::register_default_factories() {
    data_transport_factory.register_type(value::AZURE_STORAGE_BLOB, restapi_data_tranport_create);
    model_factory.register_type(value::VW, vw_model_create);
  }

  int restapi_data_tranport_create(m::i_data_transport** retval, const u::configuration& cfg, api_status* status) {
    const auto uri = cfg.get(name::MODEL_BLOB_URI, nullptr);
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
}
