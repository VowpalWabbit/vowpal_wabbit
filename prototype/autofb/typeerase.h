#pragma once

#include <unordered_map>
#include <typeinfo>
#include <typeindex>
#include <utility>

#include "type_activation.h"


using typeinfo_predicate_f = bool(*)(const std::type_info);
using dispatch_f = void(*)();

using erased_dispatch = std::pair<typeinfo_predicate_f, dispatch_f>;


struct erased_type
{
  const std::type_index tindex;
  const size_t size;
  const activator_f activator;

  template <typename T>
  bool is() const
  {
    return tindex == std::type_index{typeid(T)};
  }
};

template <typename T>
struct type 
{
public:
  inline static erased_type erase()
  {
    return erased_type{typeid(T), sizeof(T), &default_activator<T>::activate};
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

class erased_dispatch_table
{
  public:
    // TODO: SFINAE
    template <typename T, dispatch_f D>
    erased_dispatch_table& add()
    {
      auto erased = type_dispatch<T, D>::erase();
      dispatch_table[typeid(T)] = erased;
      return *this;
    }

    bool dispatch(const erased_type& type)
    {
      return dispatch(type.tindex); 
    }

    bool dispatch(const std::type_info type_info)
    {
      return dispatch(std::type_index{type_info}); 
    }

    bool dispatch(const std::type_index& type_index)
    {
      // find the dispatch based on the erased type's type_index
      auto iter = dispatch_table.find(type_index);
      if (iter == dispatch_table.end())
        return false;

      erased_dispatch dispatch = iter->second;
      dispatch.second();
      return true;
    }

  private:
    std::unordered_map<std::type_index, erased_dispatch> dispatch_table;
};

// HACK!
struct erased_data
{
  activation _data;
  erased_type _type;
  //const typesys::type_info& _type_info;
};

class erased_lvalue
{
public:
  template <typename T>
  erased_lvalue(T& data) : _type(type<T>::erase())
  {
    _data_slot = &data;
  }

  erased_type _type;

  // TODO: this really should be more similar to how property<T> works
  void* get() {}
  void set(void*&& data) {}

private:
  void* data_slot;
};

// TODO: Extend the mechanism to support output? (No, because it is just a dispatch over the types to copy them out)