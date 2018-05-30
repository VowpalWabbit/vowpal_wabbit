#include "model_mgmt.h"
#include "../restapi_data_transport.h"

using namespace reinforcement_learning::utility;

namespace reinforcement_learning { namespace model_mangement
{
  i_data_transport* blob_tranport_create(const config_collection&)
  { return new restapi_data_tranport(); }

  object_factory<i_data_transport> model_data_factory()
  {
    object_factory<i_data_transport> ret;
    ret.register_type(value::AZURE_STORAGE_BLOB, blob_tranport_create);
    return ret;
  }

  i_model* vw_model_create(const config_collection&)
  {
    return nullptr;
  }

  object_factory<i_model> model_factory()
  {
    object_factory<i_model> ret;
    ret.register_type(value::VW, vw_model_create);
    return ret;
  }
}
}
