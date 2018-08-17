#pragma once
#include <cstddef>
#include <stdint.h>

// Declare const pointer for internal linkage
namespace reinforcement_learning {
  class ranking_response;
  class api_status;
}

namespace reinforcement_learning { namespace model_management {
    class model_data {
      public:
        // Get data
        char* data() const;
        size_t data_sz() const;
        uint32_t refresh_count() const;

        void data_sz(size_t fillsz);
        void increment_refresh_count();

        // Allocate
        char* alloc(size_t desired);
        void free();

        model_data();
        ~model_data();
      private:
        char * _data = nullptr;
        size_t _data_sz = 0;
        uint32_t _refresh_count = 0;
    };

    // The i_data_transport interface provides the way to retrieve the data for a model from some source.
    class i_data_transport {
    public:
      virtual int get_data(model_data& data, api_status* status = nullptr) = 0;
      virtual ~i_data_transport() = default;
    };

    // The i_model interfaces provides the resolution from the raw model_data to a consumable object.
    class i_model {
    public:
      virtual int update(const model_data& data, api_status* status = nullptr) = 0;
      virtual int choose_rank(uint64_t rnd_seed, const char* features, ranking_response& response, api_status* status = nullptr) = 0;
      virtual ~i_model() = default;
    };
}}
