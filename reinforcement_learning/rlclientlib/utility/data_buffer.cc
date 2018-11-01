#include "data_buffer.h"
#include <string>
#include <iterator>
#include <iostream>

namespace reinforcement_learning {
  namespace utility {
    data_buffer::data_buffer() = default;

    void data_buffer::reset() { _buffer.clear(); }

    size_t data_buffer::size() const {
      return _buffer.size();
    }

    void data_buffer::remove_last() { _buffer.pop_back(); }

    void data_buffer::append(const unsigned char* data, size_t len) {
      _buffer.reserve(_buffer.size() + len);
      _buffer.insert(_buffer.end(), data, data + len);
    }

    std::vector<unsigned char> data_buffer::buffer() {
      return _buffer;
    }

    void data_buffer::reserve(size_t size){
      _buffer.reserve(size);
    }

    uint8_t* data_buffer::data() {
      return _buffer.data();
    }

    std::string data_buffer::str() const {
      return std::string(_buffer.begin(), _buffer.end());
    }

    data_buffer& data_buffer::operator<<(const std::string& cs) {
      _buffer.reserve(_buffer.size() + cs.size());
      _buffer.insert(_buffer.end(), cs.begin(), cs.end());
      return *this;
    }

    data_buffer& data_buffer::operator<<(const char* data) { return operator<<(std::string(data)); }
    data_buffer& data_buffer::operator<<(size_t rhs) { return operator<<(std::to_string(rhs)); }
    data_buffer& data_buffer::operator<<(float rhs) { return operator<<(std::to_string(rhs)); }

    buffer_factory::buffer_factory() = default;

    data_buffer* buffer_factory::operator()() const { return new data_buffer(); }
  }
}
