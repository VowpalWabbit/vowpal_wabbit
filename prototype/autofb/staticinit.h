#pragma once

// TODO: This should use the real VW types

namespace staticinit
{

template <typename T>
using static_initializer_f = T(*)();

template <typename T>
struct evidence
{
  T _value;

  evidence(static_initializer_f<T> func) : _value(func()) {
    //std::cout << "witness ctor" << std::endl;
  }

  operator T() const { return _value; }
};

template <>
struct evidence<void>
{
  evidence(static_initializer_f<void> func) {
    //std::cout << "void witness ctor" << std::endl;
    func();
  }
};

template <typename T, static_initializer_f<T> func>
struct witness
{
  evidence<T> _evidence = testify();

  inline evidence<T> get_evidence() const { return _evidence; }

  inline static evidence<T> testify()
  {
    return evidence<T>(func);
  }
};

// template <static_initializer_f<void> func>
// struct witness<void, func>
// {
//   witness() { 
//     std::cout << "witness<void, func> ctor" << std::endl;
//     func(); }

//   private: void* _;
// };

}