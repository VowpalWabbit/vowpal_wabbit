#include "model_mgmt.h"

#include <new>
#include <cstring>

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
      if (_data != nullptr) {
        delete[] _data;
        _data = nullptr;
      }
      _data_sz = 0;
    }

    model_data::model_data(model_data const& other) {
      *this = other;
    }

    model_data& model_data::operator=(model_data const& other) {
      if (this != &other) {
        // alloc will free an existing buffer, alloc the required size and set the _data_sz property.
        _data = alloc(other._data_sz);
        _refresh_count = other._refresh_count;

        // Copy the contents of the other buffer to this object.
        std::memcpy(_data, other._data, _data_sz);
      }

      return *this;
    }
}}
