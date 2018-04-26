#pragma once
#include <memory>
#include "ds_object_factory.h"

namespace decision_service { namespace model_mangement { namespace name {
      const char * MODEL_SRC          = "model.source";
      const char * MODEL_BLOB_URI     = "model.blob.uri";
      const char * MODEL_LOCAL_FILE   = "model.local.file";
      const char * MODEL              = "MODEL";
      const char * VW_CMDLINE         = "VW_CMDLINE";
}}}

namespace decision_service { namespace model_mangement { namespace value {
      const char * LOCAL_FILE         = "LOCAL_FILE";
      const char * AZURE_STORAGE_BLOB = "AZURE_STORAGE_BLOB";
      const char * VW                 = "VW";
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
