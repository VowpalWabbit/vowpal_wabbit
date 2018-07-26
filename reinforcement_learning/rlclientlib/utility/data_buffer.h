#pragma once
#include <string>
#include <vector>

namespace reinforcement_learning { namespace utility {
  class data_buffer {
  public:

    void reset();
    std::string str();
    void remove_last();

    data_buffer& operator<<(const std::string& cs);
    data_buffer& operator<<(const char*);
    data_buffer& operator<<(size_t rhs);
    data_buffer& operator<<(float rhs);

  private:
    std::vector<char> _buffer;
  };

  class buffer_factory {
  public:
    data_buffer* operator()() const;
  };
}}


