#pragma once

// Declare const pointer for internal linkage  
namespace reinforcement_learning {
  class api_status;
}

namespace reinforcement_learning { namespace model_mangement {

    struct model_data{
      char* data;
      int data_sz;
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
