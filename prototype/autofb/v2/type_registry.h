#pragma once

#include "base.h"

#include <typeinfo>
#include <typeindex>

#include "type_erase.h"

namespace type_system
{
  using name_t = const char*; // todo: std::string?
  using type_number = std::uint32_t;

  struct attribute
  {
    name_t name;
    void* data; //todo: safe deletion
  };

  using attribute_map = std::unordered_map<name_t, attribute>;

  struct descriptor_base
  {
    std::unordered_map<name_t, attribute> attributes;
  };

  struct void_t{};

  struct property_descriptor : descriptor_base
  {
    const name_t name;
    const erased_type etype;
    const bool is_vector;

    property_descriptor() : name(nullptr), etype(type<void_t>::erase()), is_vector(false)
    {
      //std::cout << "Creating property descriptor: " << "nullptr" << std::endl;
      delete (new void_t());
    }

    property_descriptor(const name_t name, const erased_type etype, const bool is_vector) : name(name), etype(etype), is_vector(is_vector) 
    {
      //std::cout << "Creating property descriptor: " << name << std::endl;
    }

    property_descriptor(const name_t name, const erased_type etype, const bool is_vector, std::initializer_list<attribute> attributes) : name(name), etype(etype), is_vector(is_vector) 
    {
      for (const auto& attribute : attributes)
      {
        this->attributes[attribute.name] = attribute;
      }
    }

    property_descriptor(const property_descriptor& other) : name(other.name), etype(other.etype), is_vector(other.is_vector) {}
    property_descriptor(property_descriptor&& other) : name(other.name), etype(other.etype), is_vector(other.is_vector) {}
  };

  struct type_descriptor : descriptor_base
  {
    using properties_t = std::unordered_map<name_t, property_descriptor>;
    using name_property = properties_t::value_type;
    using property_iter = properties_t::const_iterator;

    const name_t name;
    const type_number typeindex;
    const bool is_builtin;

    type_descriptor(const name_t name, const type_number typeindex, bool is_builtin = false)
      : name(name), typeindex(typeindex), is_builtin(is_builtin), activator(nullptr), base_index(typeindex)
    {
      std::cout << "type_descriptor: " << name << std::endl;
      this->properties.reserve(4);
    }

    type_descriptor(const name_t name, const type_number typeindex, std::initializer_list<attribute> attributes)
      : name(name), typeindex(typeindex), is_builtin(false), activator(nullptr), base_index(typeindex)
    {
      for (auto& attr : attributes)
      {
        this->attributes[attr.name] = attr;
      }
    }

    //type_descriptor(const type_descriptor&) = default;

    property_descriptor& lookup_property(const name_t name)
    {
      return properties[name];
    }

    const property_descriptor& lookup_property(const name_t name) const
    {
      return properties.at(name);
    }

    property_descriptor& define_property(const name_t name, const erased_type etype, bool is_vector)
    {
      properties.insert({ name, property_descriptor(name, etype, is_vector) });

      return properties[name];
    }

    property_descriptor& define_property(const name_t name, const erased_type etype, bool is_vector, std::initializer_list<attribute> attributes)
    {
      properties.insert({ name, property_descriptor(name, etype, is_vector, attributes) });

      return properties[name];
    }

    property_iter properties_begin() const
    {
      return properties.begin();
    }

    property_iter properties_end() const
    {
      return properties.end();
    }

    bool is_empty() const { return properties.empty(); }

    bool has_activator() const { return activator != nullptr; }
    activation activate() const { return activator(); }
    void set_activator(activator_f activator) { this->activator = activator; }

    bool has_base() const { return base_index != typeindex; }
    type_number get_base_index() const { return base_index; }
    void set_base(const type_number base_index) { this->base_index = base_index; }

  private:
    properties_t properties;
    type_number base_index;
    activator_f activator;
  };

  // singleton
  class type_registry
  {
  public:
    using types_t = std::vector<type_descriptor>;
    using type_iter = types_t::const_iterator;
    using type_map_t = std::unordered_map<name_t, type_number>;
    using std_type_map_t = std::unordered_map<std::type_index, type_number>;

    static type_registry& instance();

    type_registry();

    type_descriptor& lookup_type(const type_number typeindex) { return types[typeindex]; }
    const type_descriptor& lookup_type(const type_number typeindex) const { return types[typeindex]; }
    
    type_iter find_type(const name_t name) const
    {
      auto iter = type_map.find(name);
      if (iter == type_map.end())
      {
        return types.end();
      }

      return types.begin() + (iter->second);
    }

    type_iter find_type(const std::type_index std_index) const
    {
      auto iter = std_type_map.find(std_index);
      if (iter == std_type_map.end())
      {
        return types.end();
      }

      return types.begin() + (iter->second);
    }

    //type_descriptor& lookup_type(const name_t name) { return types[type_map[name]]; }
    //type_descriptor& lookup_type(const std::type_index std_index) { return types[std_type_map[std_index]]; }

    type_descriptor& register_type(const name_t name, erased_type etype)
    {
      return register_type_internal(name, etype, false);
    }

    type_iter types_begin() const { return types.begin(); }
    type_iter types_end() const { return types.end(); }

  private:
    type_descriptor& register_type_internal(const name_t name, erased_type etype, bool is_builtin)
    {
      type_number typeindex = types.size();
      types.push_back(type_descriptor(name, typeindex, is_builtin));
      type_map[name] = typeindex;
      std_type_map[etype.tindex] = typeindex;
      return types[typeindex];
    }

  private:
    types_t types;
    type_map_t type_map;
    std_type_map_t std_type_map;
  };

  
}