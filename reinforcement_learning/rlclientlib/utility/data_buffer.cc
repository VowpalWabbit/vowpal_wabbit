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

    buffer_factory::buffer_factory() {}

    data_buffer* buffer_factory::operator()() const { return new data_buffer(); }

    void data_buffer::append(const unsigned char* start, size_t len) {
      const auto data_len = len;
      const auto buff_sz = _buffer.size();

      if (_buffer.capacity() < buff_sz + data_len)
        _buffer.reserve(buff_sz + data_len);

      for (int i = 0; i < len; i++) {
        _buffer.push_back(start[i]);
      }
    }

    std::vector<unsigned char> data_buffer::buffer() {
      return _buffer;
    }

    std::string data_buffer::str() const {
      std::string str;
      for (auto ch : _buffer) {
        str.push_back(ch);
      }
      return str;
    }

    data_buffer& data_buffer::operator<<(const std::string& cs) {
      const auto data_len = cs.length();
      const auto buff_sz = _buffer.size();

      if (_buffer.capacity() < buff_sz + data_len)
        _buffer.reserve(buff_sz + data_len);

      const char* data = cs.c_str();
      for (int i = 0; i < data_len; i++) {
        _buffer.push_back(*((unsigned char*)(data++)));
      }
      return *this;
    }

    data_buffer& data_buffer::operator<<(const char* data) { return operator<<(std::string(data)); }
    data_buffer& data_buffer::operator<<(size_t rhs) { return operator<<(std::to_string(rhs)); }
    data_buffer& data_buffer::operator<<(float rhs) { return operator<<(std::to_string(rhs)); }
  }
}
