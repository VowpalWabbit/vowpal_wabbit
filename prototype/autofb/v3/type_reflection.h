#pragma once

#include "base.h"

#include "type_activation.h"
#include "type_erase.h"
#include "reflection.h"

namespace typesys
{

enum class field_kind
{
  scalar,
  vector,
  map
};

struct erased_field_type
{
public:
  field_kind kind;
  erased_type evalue;
  inline erased_type& ekey() { return *_ekey; }

  ~erased_field_type()
  {
    if (_ekey) 
    {
      delete _ekey;
      _ekey = nullptr;
    }
  }

  inline bool is_scalar() const { return kind == field_kind::scalar; }
  inline bool is_vector() const { return kind == field_kind::vector; }
  inline bool is_map() const { return kind == field_kind::map; }
  
  inline static erased_field_type scalar(erased_type evalue)
  {
    return erased_field_type{field_kind::scalar, evalue, nullptr};
  }

  inline static erased_field_type vector(erased_type evalue)
  {
    return erased_field_type{field_kind::vector, evalue, nullptr};
  }

  inline static erased_field_type map(erased_type ekey, erased_type evalue)
  {
    return erased_field_type{field_kind::map, evalue, new erased_type(ekey)};
  }

private:
  erased_type* _ekey;

  erased_field_type(field_kind kind, erased_type evalue, erased_type* ekey)
    : kind(kind), evalue(evalue), _ekey(ekey)
  {}
};

struct property_descriptor
{
  std::string name;
  erased_field_type eftype;
  erased_field_binder binder;
};

struct type_descriptor
{
  std::string name;
  erased_type etype;

  type_descriptor(std::string name, erased_type etype)
    : name(name), etype(etype), base_type(nullptr)
  {
  }

  // TODO: Are these needed?
  inline bool has_base_type() const { return base_type != nullptr; }
  inline const type_descriptor* get_base_type() const { return base_type; }

  inline void set_base_type_internal(type_descriptor* base_type) { this->base_type = base_type; }

  inline void register_property(property_descriptor&& prop)
  {
    _properties.emplace_back(std::move(prop));
  }

private:
  std::vector<property_descriptor> _properties;
  type_descriptor* base_type;

public:
  base::vector_view<property_descriptor> properties {_properties};
};

}