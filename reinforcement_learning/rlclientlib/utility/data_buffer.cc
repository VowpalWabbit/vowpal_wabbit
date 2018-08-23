#include "data_buffer.h"
#include <string>
#include <iterator>
#include <iostream>

namespace reinforcement_learning { namespace utility {
  data_buffer::data_buffer() = default;

  data_buffer::data_buffer(const translate_func& _translate) : translate(_translate) {}

  void data_buffer::reset() { _buffer.clear(); }

  std::string data_buffer::str() const{
    std::string retval(std::begin(_buffer), std::end(_buffer));
    return retval;
  }

  void data_buffer::remove_last() { _buffer.pop_back(); }

  buffer_factory::buffer_factory(const translate_func& _translate) : translate(_translate) {}

  data_buffer* buffer_factory::operator()() const { return new data_buffer(translate); }

  data_buffer& data_buffer::operator<<(const std::string& cs) {
    const auto data_len = cs.length();
    const auto buff_sz = _buffer.size();
    // Ensure vector is big enough
    if (_buffer.capacity() < buff_sz + data_len)
      _buffer.reserve(buff_sz + data_len);
    //workaround to be able to suppress '\n' on client side
    for (auto c : cs)
      _buffer.push_back(translate(c));
    
    return *this;
  }

  data_buffer& data_buffer::operator<<(const char* data) { return operator<<(std::string(data)); }

  data_buffer& data_buffer::operator<<(size_t rhs) { return operator<<(std::to_string(rhs)); }

  data_buffer& data_buffer::operator<<(float rhs) { return operator<<(std::to_string(rhs)); }

  translate_func::translate_func() : empty(true) {}

  translate_func::translate_func(char _src, char _dst) : empty(false), src(_src), dst(_dst) {}

  char translate_func::operator()(char c) const { return empty || c != src ? c : dst; }
}}
