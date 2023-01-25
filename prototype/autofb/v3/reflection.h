#pragma once

#include "base.h"
#include "type_erase.h"

namespace typesys
{

using erased_binder_f = bool(typesys::erased_lvalue_ref&, void*&);

struct erased_field_binder
{
  erased_type _container_type;
  erased_type _field_type;
  std::function<erased_binder_f> _binder;

  inline erased_field_binder(erased_type container_type, erased_type field_type, std::function<erased_binder_f> binder)
    : _container_type(container_type), _field_type(field_type), _binder(binder)
  {}

  inline bool try_bind(typesys::erased_lvalue_ref& container, typesys::erased_lvalue_ref*& target)
  {
    if (container._type != _container_type) return false;
    
    void* ptr = nullptr;
    if (!_binder(container, ptr)) return false;

    target = new typesys::erased_lvalue_ref{_field_type, ptr}; // is this a place for placement new?
    return true;
  }
};

template <typename C, typename T>
struct field_ptr
{
  typedef T(C::*ptr_t);

  const ptr_t ptr;

  inline T& bind(C& c) const { return c.*ptr; }

  inline const T& bind(const C& c) const { return c.*ptr; }

  inline bool try_bind(typesys::erased_lvalue_ref& elv, T*& out) const
  {
    if (!elv._type.is<C>()) return false;

    out = &bind(elv.get<C>());
    return true;
  }

  inline erased_field_binder erase_binder() const 
  { 
    // This is probably overkill, but I really do not want this object deleted out from under us.
    field_ptr<C,T> local = *this;

    return {
      type<C>::erase(), 
      type<T>::erase(),
      [local](typesys::erased_lvalue_ref& elv, void*& out) -> bool 
      { 
        return local.erased_bind(elv, out); 
      }
    };
  }

private:
  inline bool erased_bind(typesys::erased_lvalue_ref& elv, void*& out) const
  {
    T* t;
    if (!try_bind(elv, t)) return false;

    out = t;
    return true;
  }
};

template <typename C, typename T, T(C::*_ptr)>
struct field_ptr_id
{
  typedef T(C::*ptr_t);

  static constexpr ptr_t ptr = _ptr;

  inline field_ptr<C, T> operator()() const { return {ptr}; }

  static inline T& bind(C& c) { return c.*ptr; }

  static inline const T& bind(const C& c) { return c.*ptr; }

  static inline bool try_bind(typesys::erased_lvalue_ref& elv, T*& out)
  {
    if (!elv._type.is<C>()) return false;

    out = &bind(elv.get<C>());
    return true;
  }
};

}  // namespace typesys

// experimental bits
namespace typesys
{
struct i_reflector
{
  enum class kinds { scalar, vector, map };

  size_t num_scalars();
  const std::vector<std::string>& scalar_names();
  bool reflect_scalar(const char* name, erased_lvalue_ref*& out);
  
  size_t num_vectors();
  const std::vector<std::string>& vector_names();
  bool reflect_vector(const char* name, erased_vector*& out);
  
  // size_t num_maps();
  // bool reflect_map(const char* name);
};

}