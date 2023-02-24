// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/common/vw_exception.h"
#include "vw/common/vw_throw.h"

#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <vector>

namespace VW
{
namespace config
{
template <typename>
class typed_option;

class typed_option_visitor
{
public:
  virtual void visit(typed_option<uint32_t>& /*option*/){};
  virtual void visit(typed_option<uint64_t>& /*option*/){};
  virtual void visit(typed_option<int64_t>& /*option*/){};
  virtual void visit(typed_option<int32_t>& /*option*/){};
  virtual void visit(typed_option<bool>& /*option*/){};
  virtual void visit(typed_option<float>& /*option*/){};
  virtual void visit(typed_option<std::string>& /*option*/){};
  virtual void visit(typed_option<std::vector<std::string>>& /*option*/){};

  virtual ~typed_option_visitor() = default;
};

class base_option
{
public:
  base_option(std::string name, size_t type_hash) : m_name(std::move(name)), m_type_hash(type_hash) {}

  std::string m_name = "";
  size_t m_type_hash;
  std::string m_help = "";
  std::string m_short_name = "";
  bool m_keep = false;
  bool m_necessary = false;
  bool m_allow_override = false;
  bool m_hidden_from_help = false;
  std::string m_one_of_err = "";
  bool m_experimental = false;

  virtual void accept(typed_option_visitor& handler) = 0;

  virtual ~base_option() = default;

  VW_ATTR(nodiscard) const std::vector<std::string>& get_tags() const { return _tags; }
  void set_tags(std::vector<std::string> tags);

private:
  std::vector<std::string> _tags;
};

template <typename T>
class typed_option : public base_option
{
public:
  using value_type = T;

  static_assert(std::is_same<T, uint32_t>::value || std::is_same<T, uint64_t>::value ||
          std::is_same<T, int32_t>::value || std::is_same<T, int64_t>::value || std::is_same<T, float>::value ||
          std::is_same<T, std::string>::value || std::is_same<T, bool>::value ||
          std::is_same<T, std::vector<std::string>>::value,
      "typed_option<T>, T must be one of uint32_t, uint64_t, int32_t, int64_t, float, std::string, bool, "
      "std::vector<std::string");

  typed_option(const std::string& name) : base_option(name, typeid(T).hash_code()) {}

  static size_t type_hash() { return typeid(T).hash_code(); }

  void set_default_value(const value_type& value) { _m_default_value = std::make_shared<value_type>(value); }

  bool default_value_supplied() const { return _m_default_value.get() != nullptr; }

  T default_value() const
  {
    if (_m_default_value) { return *_m_default_value; }
    THROW("typed_option does not contain default value. use default_value_supplied to check if default value exists.")
  }

  bool value_supplied() const { return _m_value.get() != nullptr; }

  template <typename U>
  std::string invalid_choice_error(const U&)
  {
    return "";
  }
  std::string invalid_choice_error(const std::string& value)
  {
    std::ostringstream ss;
    ss << "Error: '" << value << "' is not a valid choice for option --" << m_name << ". Please select from {";
    std::string delim = "";
    for (const auto& choice : _m_one_of)
    {
      ss << delim << choice;
      delim = ", ";
    }
    ss << "}";
    return ss.str();
  }

  std::string invalid_choice_error(const int32_t& value) { return invalid_choice_error(std::to_string(value)); }
  std::string invalid_choice_error(const int64_t& value) { return invalid_choice_error(std::to_string(value)); }
  std::string invalid_choice_error(const uint32_t& value) { return invalid_choice_error(std::to_string(value)); }
  std::string invalid_choice_error(const uint64_t& value) { return invalid_choice_error(std::to_string(value)); }

  // Typed option children sometimes use stack local variables that are only valid for the initial set from add and
  // parse, so we need to signal when that is the case.
  typed_option& value(T value, bool called_from_add_and_parse = false)
  {
    _m_value = std::make_shared<T>(value);
    value_set_callback(value, called_from_add_and_parse);
    if (_m_one_of.size() > 0 && (_m_one_of.find(value) == _m_one_of.end()))
    {
      m_one_of_err = invalid_choice_error(value);
    }
    return *this;
  }

  T value() const
  {
    if (_m_value) { return *_m_value; }
    THROW("typed_option " << m_name << " does not contain value. use value_supplied to check if value exists.")
  }

  void set_one_of(const std::set<value_type>& one_of_set) { _m_one_of = one_of_set; }

  const std::set<value_type>& one_of() const { return _m_one_of; }

  void accept(typed_option_visitor& visitor) override { visitor.visit(*this); }

protected:
  // Allows inheriting classes to handle set values. Noop by default.
  virtual void value_set_callback(const T& /*value*/, bool /*called_from_add_and_parse*/) {}

private:
  // Would prefer to use std::optional (C++17) here but we are targeting C++11
  std::shared_ptr<T> _m_value{nullptr};
  std::shared_ptr<T> _m_default_value{nullptr};
  std::set<T> _m_one_of = {};
};

// The contract of typed_option_with_location is that the first set of the option value is written to the given
// location, otherwise it is a noop.
template <typename T>
class typed_option_with_location : public typed_option<T>
{
public:
  typed_option_with_location(const std::string& name, T& location) : typed_option<T>(name), _location{&location} {}
  virtual void value_set_callback(const T& value, bool called_from_add_and_parse) override
  {
    // This should only be done when called from add_and_parse because the location is often a stack local variable that
    // is only valid for the inital call.
    if (_location != nullptr && called_from_add_and_parse) { *_location = value; }
  }

private:
  T* _location = nullptr;
};

template <typename T>
bool operator==(const typed_option<T>& lhs, const typed_option<T>& rhs)
{
  return lhs.m_name == rhs.m_name && lhs.m_type_hash == rhs.m_type_hash && lhs.m_help == rhs.m_help &&
      lhs.m_short_name == rhs.m_short_name && lhs.m_keep == rhs.m_keep && lhs.default_value() == rhs.default_value() &&
      lhs.m_necessary == rhs.m_necessary;
}

template <typename T>
bool operator!=(const typed_option<T>& lhs, const typed_option<T>& rhs)
{
  return !(lhs == rhs);
}

inline bool operator==(const base_option& lhs, const base_option& rhs)
{
  return lhs.m_name == rhs.m_name && lhs.m_type_hash == rhs.m_type_hash && lhs.m_help == rhs.m_help &&
      lhs.m_short_name == rhs.m_short_name && lhs.m_keep == rhs.m_keep && lhs.m_necessary == rhs.m_necessary;
}

inline bool operator!=(const base_option& lhs, const base_option& rhs) { return !(lhs == rhs); }

}  // namespace config
}  // namespace VW
