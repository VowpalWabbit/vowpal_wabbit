#pragma once
#include <string>
#include <vector>

namespace reinforcement_learning { namespace utility {
  class translate_func
  {
  public:
    translate_func();
    translate_func(char _src, char _dst);

    char operator()(char c) const;

  private:
    bool empty = true;
    char src;
    char dst;
  };

  class data_buffer {
  public:
    data_buffer();
    data_buffer(const translate_func& _translate);

    void reset();
    std::string str() const;
    size_t size() const;
    void remove_last();

    data_buffer& operator<<(const std::string& cs);
    // TODO: refactor this. As here char* assumes string only
    data_buffer& operator<<(const char*);
    data_buffer& operator<<(size_t rhs);
    data_buffer& operator<<(float rhs);

    // to process binary data
    void append(const char* start, size_t len);

  private:
    std::vector<char> _buffer;
    translate_func translate;
  };

  class buffer_factory {
  public:
    buffer_factory(const translate_func& _translate);

    data_buffer* operator()() const;

  private:
    translate_func translate;
  };
}}


