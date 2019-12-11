// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include <string>
#include <utility>
#include <vector>
#include <typeinfo>
#include <memory>

namespace VW
{
namespace config
{
struct base_option
{
  base_option(std::string name, size_t type_hash) : m_name(std::move(name)), m_type_hash(type_hash) {}

  std::string m_name;
  size_t m_type_hash;
  std::string m_help = "";
  std::string m_short_name = "";
  bool m_keep = false;

  virtual ~base_option() = default;
};

template <typename T>
struct typed_option : base_option
{
  typed_option(const std::string& name, T& location) : base_option(name, typeid(T).hash_code()), m_location{location} {}

  static size_t type_hash() { return typeid(T).hash_code(); }

  typed_option& default_value(T value)
  {
    m_default_value = std::make_shared<T>(value);
    return *this;
  }

  bool default_value_supplied() { return m_default_value.get() != nullptr; }

  T default_value() { return m_default_value ? *m_default_value : T(); }

  typed_option& short_name(const std::string& short_name)
  {
    m_short_name = short_name;
    return *this;
  }

  typed_option& help(const std::string& help)
  {
    m_help = help;
    return *this;
  }

  typed_option& keep(bool keep = true)
  {
    m_keep = keep;
    return *this;
  }

  bool value_supplied() { return m_value.get() != nullptr; }

  typed_option& value(T value)
  {
    m_value = std::make_shared<T>(value);
    return *this;
  }

  T value() { return m_value ? *m_value : T(); }

  T& m_location;

 private:
  // Would prefer to use std::optional (C++17) here but we are targeting C++11
  std::shared_ptr<T> m_value{nullptr};
  std::shared_ptr<T> m_default_value{nullptr};
};

template <typename T>
typed_option<T> make_option(std::string name, T& location)
{
  return typed_option<T>(name, location);
}

struct option_group_definition
{
  option_group_definition(const std::string& name) : m_name(name) {}

  template <typename T>
  option_group_definition& add(T&& op)
  {
    m_options.push_back(std::make_shared<typename std::decay<T>::type>(op));
    return *this;
  }

  template <typename T>
  option_group_definition& operator()(T&& op)
  {
    add(std::forward<T>(op));
    return *this;
  }

  std::string m_name;
  std::vector<std::shared_ptr<base_option>> m_options;
};

struct options_i
{
  virtual void add_and_parse(const option_group_definition& group) = 0;
  virtual bool was_supplied(const std::string& key) = 0;
  virtual std::string help() = 0;

  virtual std::vector<std::shared_ptr<base_option>> get_all_options() = 0;
  virtual std::shared_ptr<base_option> get_option(const std::string& key) = 0;

  virtual void insert(const std::string& key, const std::string& value) = 0;
  virtual void replace(const std::string& key, const std::string& value) = 0;

  template <typename T>
  typed_option<T>& get_typed_option(const std::string& key)
  {
    base_option& base = *get_option(key);
    if (base.m_type_hash != typed_option<T>::type_hash())
    {
      throw std::bad_cast();
    }

    return dynamic_cast<typed_option<T>&>(base);
  }

  // Will throw if any options were supplied that do not having a matching argument specification.
  virtual void check_unregistered() = 0;

  virtual ~options_i() = default;
};

struct options_serializer_i
{
  virtual void add(base_option& argument) = 0;
  virtual std::string str() = 0;
  virtual const char* data() = 0;
  virtual size_t size() = 0;
};

template <typename T>
bool operator==(typed_option<T>& lhs, typed_option<T>& rhs)
{
  return lhs.m_name == rhs.m_name && lhs.m_type_hash == rhs.m_type_hash && lhs.m_help == rhs.m_help &&
      lhs.m_short_name == rhs.m_short_name && lhs.m_keep == rhs.m_keep && lhs.default_value() == rhs.default_value();
}

template <typename T>
bool operator!=(typed_option<T>& lhs, typed_option<T>& rhs)
{
  return !(lhs == rhs);
}

bool operator==(const base_option& lhs, const base_option& rhs);
bool operator!=(const base_option& lhs, const base_option& rhs);

inline bool operator==(const base_option& lhs, const base_option& rhs)
{
  return lhs.m_name == rhs.m_name && lhs.m_type_hash == rhs.m_type_hash && lhs.m_help == rhs.m_help &&
      lhs.m_short_name == rhs.m_short_name && lhs.m_keep == rhs.m_keep;
}

inline bool operator!=(const base_option& lhs, const base_option& rhs) { return !(lhs == rhs); }

}  // namespace config
}  // namespace VW
