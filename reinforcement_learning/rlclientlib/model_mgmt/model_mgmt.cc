#include "model_mgmt.h"

#include <new>

namespace reinforcement_learning {
  namespace model_management {

    model_data::model_data() = default;

    model_data::~model_data() {
      free();
    }

    char* model_data::data() const {
      return _data;
    }

    void model_data::increment_refresh_count() {
      ++_refresh_count;
    }

    size_t model_data::data_sz() const {
      return _data_sz;
    }

    uint32_t model_data::refresh_count() const {
      return _refresh_count;
    }

    void model_data::data_sz(const size_t fillsz) {
      _data_sz = fillsz;
    }

    char* model_data::alloc(const size_t desired) {
      free();
      _data = new(std::nothrow) char[desired];
      _data_sz = (_data == nullptr) ? 0 : desired;
      return _data;
    }

    void model_data::free() {
      delete[] _data;
      _data = nullptr;
      _data_sz = 0;
    }
}}
