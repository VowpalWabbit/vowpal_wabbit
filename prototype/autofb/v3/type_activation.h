#pragma once

#include "base.h"
//#include "meta.h"

// Provides functionality to "activate" known types by providing a paired ctor/dtor, which can instantiate (and
// safely destruct, when appropriate), type-erased containers of an instance of the type.

// It creates a "type-smuggled" equivalent to unique_ptr: "activation". This is the key to being able to perform
// data binding, because it enabled creation and type-erased access to pointers, in a way that can be made memory
// safe (up to the consumer not trying to break through the guardrails)

namespace typesys
{
using init_f_raw = void*(*)();
using destroy_f_raw = void(*)(void*);

template <typename T>
using init_function_t = T*(*)();

// template <typename T>
// using is_init_functor = std::is_same<init_function_t<T>, decltype(T::operator())>;

template <typename T>
using destroy_function_t = void(*)(T*);

// template <typename T>
// using is_destroy_functor = std::is_same<destroy_function_t<T>, decltype(T::operator())>;

struct erased_destroy_f
{
  
};

struct activation
{
public:
  template <typename T>
  activation(T*&& ptr, destroy_function_t<T> destroy) 
    : _value{static_cast<void*>(ptr), 
             std::move([destroy](void* ptr)
             {
               destroy(static_cast<T*>(ptr));
             })} 
  {}

  activation(activation&& other) : _value{std::move(other._value)} {};
  activation(const activation& other) = delete;

  //override assignment operators
  template <typename T, typename Deleter = std::default_delete<T>>
  activation& operator=(std::unique_ptr<T, Deleter>&& unwrapped)
  {
    // type erase the incoming pointer, and store it in the activation
    _value = std::unique_ptr<void, destroy_f>(unwrapped.release(), [](void* ptr) { Deleter{}(static_cast<T*>(ptr)); });
  }

  activation& operator=(activation&& other) { _value = std::move(other._value); return *this; }
  activation& operator=(const activation& other) = delete;

  // override the comparison - this one is trivial, this is eq IFF the underlying pointer is eq
  // activation equality is defined as activation sameness, since the concern here is not the ontic
  // equality of values, but whether they are referring to the same object.
  // Incidentally, this should return false unless we are comparing the actual same object.
  bool operator==(const activation& other) const { return _value.get() == other._value.get(); }
  bool operator==(const void* other) const { return _value.get() == other; }

public:
  // TODO: I really question whether we should *ever* call this function: It returns a reference
  // but the receiver is expected to delete the object. This is a recipe for disaster.
  // template <typename T>
  // T& detach()
  // {
  //   return *detach(); // why is the compiler happy with this without a cast?!
  // }
  void* move_out()
  {
    return _value.release();
  }

  template <typename T>
  T& get() const 
  {
    return *static_cast<T*>(_value.get()); 
  }

  void* get() const
  {
    return _value.get();
  }

private:
  std::unique_ptr<void, std::function<void(void*)>> _value;
};

template <typename T, init_function_t<T> init_f = default_init<T>, destroy_function_t<T> destroy_f = default_destroy<T>>
class activator
{
public:
  static activation activate()
  {
    return activation(init_f(), destroy_f);
  }
};

template <typename T, typename = std::enable_if_t<std::is_default_constructible<T>::value>>
T* default_init()
{
  return new T();
}

template <typename T>
void default_destroy(T* ptr)
{
  std::default_delete<T>{}(ptr);
}

using activator_f = activation(*)();
}
