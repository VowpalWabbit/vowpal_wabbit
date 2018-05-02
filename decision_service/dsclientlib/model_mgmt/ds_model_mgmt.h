#pragma once
#include "../utility/ds_object_factory.h"

// Declare const pointer for internal linkage  
namespace decision_service {
  using str_const = const char * const;
}

namespace decision_service { namespace model_mangement { namespace name {
      str_const MODEL_SRC          = "model.source";
      str_const MODEL_BLOB_URI     = "model.blob.uri";
      str_const MODEL_LOCAL_FILE   = "model.local.file";
      str_const MODEL              = "MODEL";
      str_const VW_CMDLINE         = "VW_CMDLINE";
}}}

namespace decision_service { namespace model_mangement { namespace value {
      str_const LOCAL_FILE         = "LOCAL_FILE";
      str_const AZURE_STORAGE_BLOB = "AZURE_STORAGE_BLOB";
      str_const VW                 = "VW";
}}}

namespace decision_service { namespace model_mangement {

    struct model_data{
      char* data;
      int data_sz;
    };

    class i_data_transport{
    public:
      virtual model_data get_data() = 0;
      virtual ~i_data_transport() {}
    };

    class i_model {
    public:
      virtual model_data init(model_data& data) = 0;
      virtual int choose(char* features, int actions[]) = 0;
      virtual ~i_model() {}
    };


    utility::object_factory<i_data_transport> model_data_factory();
    utility::object_factory<i_model> model_factory();
}
}
