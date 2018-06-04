#pragma once
#include <cstdint>

// Declare const pointer for internal linkage  
namespace reinforcement_learning {
  class api_status;
}

namespace reinforcement_learning { namespace model_management {

    struct model_data{
      model_data();

      const char * data;
      uint64_t data_sz;
      int data_refresh_count;
    };

    class i_data_transport{
    public:
      virtual int get_data(model_data& data, api_status* status = nullptr) = 0;
      virtual ~i_data_transport() {}
    };

    class i_model {
    public:
      virtual int init(model_data& data, api_status* status = nullptr) = 0;
      virtual int choose_rank(int& action, char* features, int actions[], api_status* status = nullptr) = 0;
      virtual ~i_model() {}
    };
}
}
