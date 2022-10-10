#pragma once

#include <typeinfo>
#include <utility>

using typeinfo_predicate_f = bool(*)(const std::type_info);
using dispatch_f = void(*)();

using erased_dispatch = std::pair<typeinfo_predicate_f, dispatch_f>;

struct erased_type
{
  const std::type_info tinfo;
  const size_t size;

  template <typename T>
  bool is() const
  {
    return tinfo == typeid(T);
  }


};

template <typename T>
struct type 
{
public:
  inline static erased_type erase()
  {
    return erased_type{typeid(T), sizeof(T)};
  }
};

template <typename T, dispatch_f D>
struct type_dispatch
{
  static erased_dispatch erase()
  {
    return { &is_match, D };
  }

  static bool is_match(const std::type_info info)
  {
    return info == typeid(T);
  }
};

// TODO: SFINAE
template <typename H, typename... T> // H and Ts are all supposed to be type_dispatch objects
struct dispatch_list : dispatch_list<T...>
{
  
};

// TODO: Extend the mechanism to support output? (No, because it is just a dispatch over the types to copy them out)