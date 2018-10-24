#pragma once
#include <string>
#include <vector>

namespace reinforcement_learning { namespace utility {

  class data_buffer {
  public:
    data_buffer();
    void reset();
    std::vector<unsigned char> buffer();
    size_t size() const;
    void remove_last();
    void append(const unsigned char* start, size_t len);

  private:
    std::vector<unsigned char> _buffer;
  };

  class buffer_factory {
  public:
    buffer_factory();
    data_buffer* operator()() const;
  };
}}


