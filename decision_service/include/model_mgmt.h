#pragma once
#include <cstdint>

// Declare const pointer for internal linkage  
namespace reinforcement_learning {
class ranking_response;
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
      virtual ~i_data_transport() = default;
    };

    class i_model {
    public:
      virtual int update(const model_data& data, api_status* status = nullptr) = 0;
      virtual int choose_rank(const char* rnd_seed, const char* features, ranking_response& response, api_status* status = nullptr) = 0;
      virtual ~i_model() = default;
    };
}
}
