#pragma once

#include <iostream>
#include <unordered_map>
#include <typeinfo>
#include <typeindex>
#include <utility>
#include <algorithm>
#include <functional>

#include "type_activation.h"

using typeinfo_predicate_f = bool(*)(const std::type_info);

template <typename...Ts>
using dispatch_f = void(*)(Ts...args);

using empty_effect_f = dispatch_f<>;

template <typename...Ts>
using erased_dispatch = std::pair<typeinfo_predicate_f, dispatch_f<Ts...>>;

using erased_empty_effect = erased_dispatch<>;

struct erased_type
{
  std::type_index tindex;
  size_t size;
  activator_f activator;

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

template <typename T, typename...Ts>
struct type_dispatch
{
  template <dispatch_f<Ts...> D>
  static erased_dispatch<Ts...> erase()
  {
    return { &is_match, D };
  }

  static bool is_match(const std::type_info info)
  {
    return info == typeid(T);
  }
};

template <typename... Ts>
class erased_dispatch_table
{
  using erased_dispatch_t = erased_dispatch<Ts...>;

  public:
    // TODO: SFINAE
    template <typename T, dispatch_f<Ts...> D>
    erased_dispatch_table& add()
    {
      auto erased = type_dispatch<T, Ts...>::erase<D>();
      dispatch_table[typeid(T)] = erased;
      return *this;
    }

    template <typename T>
    static bool is_match(const std::type_info info)
    {
      return info == typeid(T);
    }

    template <typename T>
    erased_dispatch_table& add(dispatch_f<Ts...> D)
    {
      erased_dispatch erased = { &is_match<T>, D };
      dispatch_table[typeid(T)] = erased;
      return *this;
    }

    bool dispatch(const erased_type& type, Ts...args)
    {
      return dispatch(type.tindex, std::forward<Ts>(args)...); 
    }

    bool dispatch(const std::type_info type_info, Ts... args)
    {
      return dispatch(std::type_index{type_info}, std::forward<Ts>(args)...); 
    }

    bool dispatch(const std::type_index& type_index, Ts...args)
    {
      // find the dispatch based on the erased type's type_index
      auto iter = dispatch_table.find(type_index);
      if (iter == dispatch_table.end())
        return false;

      erased_dispatch<Ts...> dispatch = iter->second;
      dispatch.second(std::forward<Ts>(args)...);
      return true;
    }

  private:
    std::unordered_map<std::type_index, erased_dispatch_t> dispatch_table;
};

using erased_effect_table = erased_dispatch_table<>;

// HACK!
struct erased_data
{
  activation _data;
  erased_type _type;
  //const typesys::type_info& _type_info;

  template <typename T>
  static erased_data erase(T data, destroy_f destroy = &default_destroyer<T>::destroy)
  {
    return erased_data{ activation{data, destroy}, type<T>::erase() };
  }
};


struct ref
{
  void* _r;

  ref(const ref& r)
  {
    _r = r._r;
  }

  ref(ref&& r)
  {
    _r = r._r;
  }

  // the enable_if does a very important job here - it prevents the compiler from
  // treating the template as a valid copy constructor for ref. Without it, we
  // double-wrap whenever we copy, leading to "fun" bugs.
  template <typename T, class = std::enable_if<!std::is_same<T, ref>::value>::type>
  ref(T& r) : _r(&r) 
  {
    // std::cout << "ref(T& r) : r(&r) @" << this->_r << std::endl << '\t';
    // std::cout << *(float*)(&r) << " | " << get<float>() << std::endl << '\t';

    // if (std::is_same<T, float>::value)
    // {
    //   std::cout << "is float" << std::endl;
    // }
    // else if (std::is_same<T, ref>::value)
    // {
    //   std::cout << "is ref" << std::endl;
    // }
    // else
    // {
    //   std::cout << "is not float or ref" << std::endl;
    // }
  }

  // void* ctor
  // ref(void*& r) : _r(&r) 
  // {
  // }

  //getter
  template <typename T>
  T& get() const 
  { 
    // std::cout << "getting @" << _r << std::endl; 
    return *static_cast<T*>(_r); 
  }

  //getter as void*
  void*& get() const { return get<void*>(); }

  //setter
  template <typename T>
  T& operator=(T& t) { return *static_cast<T*>(_r) = t; }

  //setter as void*
  //void* operator=(void* t) { return *static_cast<void**>(r); }
};

class erased_lvalue
{
public:
  erased_lvalue(erased_type etype, ref slot) : _type{etype}, slot{slot}
  {
  }

  erased_type _type;

  // TODO: this really should be more similar to how property<T> works
  void*& get() const
  { 
    return (slot.get()); 
  }

  template <typename T>
  T& get() const
  {
    return slot.get<T>();
  }

  template <typename T>
  void set(T t)
  {
    slot.get<T>() = t;
  }

  void set(void* data) 
  { 
    slot = data;
  }

private:
  ref slot;
};

struct i_vector
{
  using data_f = std::function<void*(void)>;
  data_f _data_f;

  using size_f = std::function<size_t(void)>;
  size_f _size_f;

  using assign_f = std::function<void(const void*, const void*)>;
  assign_f _assign_f;

  using push_back_f = std::function<void(activation)>;
  push_back_f _push_back_f;
};

struct erased_vector
{
  erased_type _type;
  i_vector _vector;

  void* data() const { return _vector._data_f(); }
  size_t size() const { return _vector._size_f(); }

  template <typename T>
  void reduce(dispatch_f<const T*, size_t> dispatch) const
  {
    T* tdata = static_cast<T*>(data());
    dispatch(tdata, size());
  }

  template <typename T>
  void reduce(std::function<void(const T*, size_t)> dispatch) const
  {
    T* tdata = static_cast<T*>(data());
    dispatch(tdata, size());
  }

  void copy_to(void* dest, size_t dest_size) const
  {
    const size_t count = dest_size / _type.size;

    if (count < size())
    {
      // TODO: report size mismatch
    }

    std::memcpy(dest, data(), count * _type.size);
  }

  void push_back(void* element)
  {
    
  }

  void assign_from(const void* src_begin, const void* src_end)
  {
    _vector._assign_f(src_begin, src_end);
  }
};

template <typename T>
struct vtype
{
  static erased_vector erase(std::vector<T>& vec)
  {
    i_vector v = {
      [&vec]() -> void* { return vec.data(); }, 
      [&vec]() -> size_t { return vec.size(); },
      [&vec](const void* begin, const void* end) { vec.assign(static_cast<const T*>(begin), static_cast<const T*>(end)); },
      [&vec](activation element) { vec.push_back(std::move(element.get<T>())); }
    };

    return erased_vector{ type<T>::erase(), v };
  }
};


