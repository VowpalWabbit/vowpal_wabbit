#pragma once

#include <memory>

using init_f = void*(*)();
using destroy_f = void(*)(void*);

class activation
{
public:
  activation(void*&& ptr, destroy_f destroy) : _value(ptr, destroy) {}

private:
  std::unique_ptr<void, destroy_f> _value;
};

template <init_f I, destroy_f D>
class activator
{
public:
  static activation activate()
  {
    return activation(std::move(I()), D);
  }
};

using activator_f = activation(*)();

template <typename T, typename = std::enable_if_t<std::is_default_constructible<T>::value>>
void* default_init()
{
  return new T();
}

// template <typename T>
// void* default_init() = delete;

// SFINAE error for non-default constructible types
// template <typename T>
// void* default_init()
// {
//   static_assert(std::is_default_constructible<T>::value, "Cannot default initialize type that is not trivially constructible");
//   return nullptr;
// }

template <typename T, typename = std::enable_if<!std::is_array<T>::value>::type>
void default_destroy(void* ptr)
{
  delete static_cast<T*>(ptr);
}

// TODO: 
// template <typename T, typename = std::enable_if<!std::is_array<T>::value>::type>
// void default_destroy(void* ptr)
// {
//   delete static_cast<std::remove_extent<>>(ptr);
// }


template <typename T>
using default_activator = activator<&default_init<T>, &default_destroy<T>>;

