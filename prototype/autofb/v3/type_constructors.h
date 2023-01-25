#include "base.h"
#include "type_reflection.h"

namespace typesys
{
template <typename T, const field_kind kind>
struct PropBase
{
  using field_kind = std::integral_constant<field_kind, kind>;
  using storage_type = T;

protected:
  T val;
};

template <typename T>
struct Prop : public PropBase<T, field_kind::scalar>
{
  using value_type = T;

  static erased_field_type& eftype()
  {
    static auto _eftype = erased_field_type::scalar(type<T>::erase());
    return _eftype;
  }

  Prop() = default;
  Prop(const T& val) : PropBase<T>{val} {}
  Prop(T&& val) : PropBase<T>{std::move(val)} {}

  inline erased_lvalue_ref reflect() { erased_lvalue_ref{eftype().evalue, ref{val}}; }

  using PropBase<T, typesys::field_kind::scalar>::val;

  // implicit cast to T&
  inline operator T&() { return val; }
  inline operator const T&() const { return val; }
};

template <typename T>
struct Vec : public PropBase<std::vector<T>, field_kind::vector>
{
  using value_type = T;
  using iterator = typename std::vector<T>::iterator;
  using const_iterator = typename std::vector<T>::const_iterator;

  Vec() = default;
  Vec(const std::vector<T>& val) : Prop<std::vector<T>>{val} {}
  Vec(std::vector<T>&& val) : Prop<std::vector<T>>{std::move(val)} {}

  iterator begin() { return val.begin(); }
  iterator end() { return val.end(); }
  const_iterator begin() const { return val.begin(); }
  const_iterator end() const { return val.end(); }

  inline erased_vector reflect() { return vtype<T>::erase(val); }
};

template <typename K, typename V>
struct UMap : public PropBase<std::unordered_map<K, V>, field_kind::map>
{
  using key_type = K;
  using value_type = V;

  UMap() = default;
  UMap(const std::unordered_map<K, V>& val) : Prop<std::unordered_map<K, V>>{val} {}
  UMap(std::unordered_map<K, V>&& val) : Prop<std::unordered_map<K, V>>{std::move(val)} {}

  
};

template <typename C, typename T, template<typename _> typename Wrapper = Prop>
struct PropertyBuilder
{
  struct property_desc
  {
    std::string _name;
    typesys::erased_type _type;
    typesys::erased_field_binder _binder;
  };

  using field_ptr_t = Wrapper<T>(C::*);

  property_desc _desc;

  PropertyBuilder()
  {
    _desc._type = type<T>::erase();
  }

  PropertyBuilder& with_name(const char* name)
  {
    _desc._name = name;
    return *this;
  }

  PropertyBuilder& with_field_ptr(field_ptr_t ptr)
  {
    _desc._binder = typesys::field_ptr<C, T>{ptr}.get_binder();
    return *this;
  }

  // implicit cast operator to Prop<T>
  operator Prop<T>() const
  {
    // how do we ensure we can get the property_desc from somewhere?

    return Prop<T>{};
  }
};

template <typename K>
struct MapHelper
{
  template <typename V>
  using Wrapper = UMap<K, V>;
};

template <typename C, typename K, typename V>
using MapPropertyBuilder = PropertyBuilder<C, UMap<K, V>, MapHelper<K>::template Wrapper>;

}  // namespace typesys