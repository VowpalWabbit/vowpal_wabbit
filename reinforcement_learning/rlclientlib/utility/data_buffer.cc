#include "data_buffer.h"
#include <string>
#include <iterator>
#include <iostream>

namespace reinforcement_learning { namespace utility {
  void data_buffer::reset() { _buffer.clear(); }

  std::string data_buffer::str() const{
    std::string retval(std::begin(_buffer), std::end(_buffer));
    return retval;
  }

  void data_buffer::remove_last() { _buffer.pop_back(); }

  data_buffer* buffer_factory::operator()() const { return new data_buffer(); }

  data_buffer& data_buffer::operator<<(const std::string& cs) {
    const auto data_len = cs.length();
    const auto buff_sz = _buffer.size();
    // Ensure vector is big enough
    if (_buffer.capacity() < buff_sz + data_len)
      _buffer.reserve(buff_sz + data_len);
    std::copy(cs.begin(), cs.end(), std::back_inserter(_buffer));
    return *this;
  }

  data_buffer& data_buffer::operator<<(const char* data) { return operator<<(std::string(data)); }

  data_buffer& data_buffer::operator<<(size_t rhs) { return operator<<(std::to_string(rhs)); }

  data_buffer& data_buffer::operator<<(float rhs) { return operator<<(std::to_string(rhs)); }
}}
