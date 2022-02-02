// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include <string>
#include <utility>
#include <vector>
#include <map>
#include <set>
#include <typeinfo>
#include <memory>
#include <unordered_set>
#include <sstream>
#include <cstdio>
#include <fmt/format.h>
#include <iostream>
#include <type_traits>

#include "options_types.h"
#include "vw_exception.h"
#include "io/logger.h"

namespace VW
{
namespace config
{
struct base_option;

// option_builder decouples the specific type of the option and the interface
// for building it. It handles generating a shared_ptr at the end of the
// base_option type which the options framework takes as input.
// Therefore, T must satisfy:
// - Inherit from base_option
// - Statically expose a type value_type
// - Have a function set_default_value which accepts a single parameter const value_type&
// - For nearly all purposes T should be typed_option<value_type> or a subclass
//   of this.
template <typename T>
struct option_builder
{
  template <typename... Args>
  option_builder(Args&&... args) : m_option_obj(std::forward<Args>(args)...)
  {
  }

  option_builder& default_value(const typename T::value_type& value)
  {
    m_option_obj.set_default_value(value);
    return *this;
  }

  option_builder& short_name(const std::string& short_name)
  {
    m_option_obj.m_short_name = short_name;
    return *this;
  }

  template <typename U>
  std::string help_one_of(const std::string&, const std::set<U>&)
  {
    THROW("Error: cannot handle non-string or arithmetic types in one_of().");
  }
  std::string help_one_of(const std::string& help, const std::set<std::string>& s)
  {
    return fmt::format("{}. Choices: {{{}}}", help, fmt::join(s, ", "));
  }
  std::string help_one_of(const std::string& help, const std::set<int32_t>& s)
  {
    return fmt::format("{}. Choices: {{{}}}", help, fmt::join(s, ", "));
  }
  std::string help_one_of(const std::string& help, const std::set<int64_t>& s)
  {
    return fmt::format("{}. Choices: {{{}}}", help, fmt::join(s, ", "));
  }
  std::string help_one_of(const std::string& help, const std::set<uint32_t>& s)
  {
    return fmt::format("{}. Choices: {{{}}}", help, fmt::join(s, ", "));
  }
  std::string help_one_of(const std::string& help, const std::set<uint64_t>& s)
  {
    return fmt::format("{}. Choices: {{{}}}", help, fmt::join(s, ", "));
  }

  option_builder& help(const std::string& help)
  {
    m_option_obj.m_help = m_option_obj.one_of().empty() ? help : help_one_of(help, m_option_obj.one_of());
    return *this;
  }

  option_builder& keep(bool keep = true)
  {
    m_option_obj.m_keep = keep;
    return *this;
  }

  option_builder& necessary(bool necessary = true)
  {
    m_option_obj.m_necessary = necessary;
    return *this;
  }

  option_builder& one_of(std::set<typename T::value_type> args)
  {
    m_option_obj.set_one_of(args);
    return *this;
  }

  option_builder& allow_override(bool allow_override = true)
  {
    if (!is_scalar_option_type<typename T::value_type>::value)
    { THROW("allow_override can only apply to scalar option types.") }
    m_option_obj.m_allow_override = allow_override;
    return *this;
  }

  static std::shared_ptr<base_option> finalize(option_builder&& option)
  {
    return std::make_shared<T>(std::move(option.m_option_obj));
  }

private:
  T m_option_obj;
};

template <typename>
struct typed_option;

struct typed_option_visitor
{
  virtual void visit(typed_option<uint32_t>& /*option*/){};
  virtual void visit(typed_option<uint64_t>& /*option*/){};
  virtual void visit(typed_option<int64_t>& /*option*/){};
  virtual void visit(typed_option<int32_t>& /*option*/){};
  virtual void visit(typed_option<bool>& /*option*/){};
  virtual void visit(typed_option<float>& /*option*/){};
  virtual void visit(typed_option<std::string>& /*option*/){};
  virtual void visit(typed_option<std::vector<std::string>>& /*option*/){};
};

struct base_option
{
  base_option(std::string name, size_t type_hash) : m_name(std::move(name)), m_type_hash(type_hash) {}

  std::string m_name = "";
  size_t m_type_hash;
  std::string m_help = "";
  std::string m_short_name = "";
  bool m_keep = false;
  bool m_necessary = false;
  bool m_allow_override = false;
  std::string m_one_of_err = "";

  virtual void accept(typed_option_visitor& handler) = 0;

  virtual ~base_option() = default;
};

template <typename T>
struct typed_option : base_option
{
  using value_type = T;

  static_assert(std::is_same<T, uint32_t>::value || std::is_same<T, uint64_t>::value ||
          std::is_same<T, int32_t>::value || std::is_same<T, int64_t>::value || std::is_same<T, float>::value ||
          std::is_same<T, std::string>::value || std::is_same<T, bool>::value ||
          std::is_same<T, std::vector<std::string>>::value,
      "typed_option<T>, T must be one of uint32_t, uint64_t, int32_t, int64_t, float, std::string, bool, "
      "std::vector<std::string");

  typed_option(const std::string& name) : base_option(name, typeid(T).hash_code()) {}

  static size_t type_hash() { return typeid(T).hash_code(); }

  void set_default_value(const value_type& value) { m_default_value = std::make_shared<value_type>(value); }

  bool default_value_supplied() const { return m_default_value.get() != nullptr; }

  T default_value() const
  {
    if (m_default_value) { return *m_default_value; }
    THROW("typed_option does not contain default value. use default_value_supplied to check if default value exists.")
  }

  bool value_supplied() const { return m_value.get() != nullptr; }

  template <typename U>
  std::string invalid_choice_error(const U&)
  {
    return "";
  }
  std::string invalid_choice_error(const std::string& value)
  {
    return fmt::format("Error: '{}' is not a valid choice for option --{}. Please select from {{{}}}", value, m_name,
        fmt::join(m_one_of, ", "));
  }
  std::string invalid_choice_error(const int32_t& value) { return invalid_choice_error(std::to_string(value)); }
  std::string invalid_choice_error(const int64_t& value) { return invalid_choice_error(std::to_string(value)); }
  std::string invalid_choice_error(const uint32_t& value) { return invalid_choice_error(std::to_string(value)); }
  std::string invalid_choice_error(const uint64_t& value) { return invalid_choice_error(std::to_string(value)); }

  // Typed option children sometimes use stack local variables that are only valid for the initial set from add and
  // parse, so we need to signal when that is the case.
  typed_option& value(T value, bool called_from_add_and_parse = false)
  {
    m_value = std::make_shared<T>(value);
    value_set_callback(value, called_from_add_and_parse);
    if (m_one_of.size() > 0 && (m_one_of.find(value) == m_one_of.end())) { m_one_of_err = invalid_choice_error(value); }
    return *this;
  }

  T value() const
  {
    if (m_value) { return *m_value; }
    THROW("typed_option does not contain value. use value_supplied to check if value exists.")
  }

  void set_one_of(const std::set<value_type>& one_of_set) { m_one_of = one_of_set; }

  const std::set<value_type>& one_of() const { return m_one_of; }

  void accept(typed_option_visitor& visitor) override { visitor.visit(*this); }

protected:
  // Allows inheriting classes to handle set values. Noop by default.
  virtual void value_set_callback(const T& /*value*/, bool /*called_from_add_and_parse*/) {}

private:
  // Would prefer to use std::optional (C++17) here but we are targeting C++11
  std::shared_ptr<T> m_value{nullptr};
  std::shared_ptr<T> m_default_value{nullptr};
  std::set<T> m_one_of = {};
};

// The contract of typed_option_with_location is that the first set of the option value is written to the given
// location, otherwise it is a noop.
template <typename T>
struct typed_option_with_location : typed_option<T>
{
  typed_option_with_location(const std::string& name, T& location) : typed_option<T>(name), m_location{&location} {}
  virtual void value_set_callback(const T& value, bool called_from_add_and_parse) override
  {
    // This should only be done when called from add_and_parse because the location is often a stack local variable that
    // is only valid for the inital call.
    if (m_location != nullptr && called_from_add_and_parse) { *m_location = value; }
  }

private:
  T* m_location = nullptr;
};

template <typename T>
option_builder<typed_option_with_location<T>> make_option(const std::string& name, T& location)
{
  return typed_option_with_location<T>(name, location);
}

struct option_group_definition;

struct options_i
{
  virtual void add_and_parse(const option_group_definition& group) = 0;
  virtual void tint(const std::string& reduction_name) = 0;
  virtual void reset_tint() = 0;
  virtual bool add_parse_and_check_necessary(const option_group_definition& group) = 0;
  virtual bool was_supplied(const std::string& key) const = 0;
  virtual std::string help(const std::vector<std::string>& enabled_reductions) const = 0;

  virtual std::vector<std::shared_ptr<base_option>> get_all_options() = 0;
  virtual std::vector<std::shared_ptr<const base_option>> get_all_options() const = 0;
  virtual std::shared_ptr<base_option> get_option(const std::string& key) = 0;
  virtual std::shared_ptr<const base_option> get_option(const std::string& key) const = 0;
  virtual std::map<std::string, std::vector<option_group_definition>> get_collection_of_options() const = 0;

  virtual void insert(const std::string& key, const std::string& value) = 0;
  virtual void replace(const std::string& key, const std::string& value) = 0;
  virtual std::vector<std::string> get_positional_tokens() const { return std::vector<std::string>(); }

  template <typename T>
  typed_option<T>& get_typed_option(const std::string& key)
  {
    base_option& base = *get_option(key);
    if (base.m_type_hash != typed_option<T>::type_hash()) { throw std::bad_cast(); }

    return dynamic_cast<typed_option<T>&>(base);
  }

  template <typename T>
  const typed_option<T>& get_typed_option(const std::string& key) const
  {
    const base_option& base = *get_option(key);
    if (base.m_type_hash != typed_option<T>::type_hash()) { throw std::bad_cast(); }

    return dynamic_cast<const typed_option<T>&>(base);
  }

  // Will throw if any options were supplied that do not having a matching argument specification.
  virtual void check_unregistered(VW::io::logger& logger) = 0;

  virtual ~options_i() = default;
};

struct option_group_definition
{
  // add second parameter for const string short name
  option_group_definition(const std::string& name) : m_name(name + " Options") {}

  template <typename T>
  option_group_definition& add(option_builder<T>&& op)
  {
    auto built_option = option_builder<T>::finalize(std::move(op));
    m_options.push_back(built_option);
    if (built_option->m_necessary) { m_necessary_flags.insert(built_option->m_name); }
    return *this;
  }

  template <typename T>
  option_group_definition& add(option_builder<T>& op)
  {
    return add(std::move(op));
  }

  // will check if ALL of 'necessary' options were suplied
  bool check_necessary_enabled(const options_i& options) const
  {
    if (m_necessary_flags.size() == 0) return false;

    bool check_if_all_necessary_enabled = true;

    for (const auto& elem : m_necessary_flags) { check_if_all_necessary_enabled &= options.was_supplied(elem); }

    return check_if_all_necessary_enabled;
  }

  // will check if one_of condition is met for all options
  bool check_one_of() const
  {
    for (const auto& option : m_options)
    {
      if (!option->m_one_of_err.empty()) { THROW(option->m_one_of_err); }
    }
    return true;
  }

  template <typename T>
  option_group_definition& operator()(T&& op)
  {
    add(std::forward<T>(op));
    return *this;
  }

  std::string m_name = "";
  std::unordered_set<std::string> m_necessary_flags;
  std::vector<std::shared_ptr<base_option>> m_options;
};

struct options_name_extractor : options_i
{
  std::string generated_name;
  std::set<std::string> m_added_help_group_names;

  void add_and_parse(const option_group_definition&) override
  {
    THROW("add_parse_and_check_necessary() should be used inside a reduction setup instead.");
  };

  bool add_parse_and_check_necessary(const option_group_definition& group) override
  {
    if (group.m_necessary_flags.empty()) { THROW("reductions must specify at least one .necessary() option"); }

    if (m_added_help_group_names.count(group.m_name) == 0) { m_added_help_group_names.insert(group.m_name); }
    else
    {
      THROW("repeated option_group_definition name: " + group.m_name);
    }

    generated_name.clear();

    for (const auto& opt : group.m_options)
    {
      if (opt->m_necessary)
      {
        if (generated_name.empty())
          generated_name += opt->m_name;
        else
          generated_name += "_" + opt->m_name;
      }
    }

    return false;
  };

  bool was_supplied(const std::string&) const override { return false; };

  void tint(const std::string&) override { THROW("options_name_extractor does not implement this method"); };

  void reset_tint() override { THROW("options_name_extractor does not implement this method"); };

  std::string help(const std::vector<std::string>&) const override
  {
    THROW("options_name_extractor does not implement this method");
  };

  void check_unregistered(VW::io::logger& /* logger */) override
  {
    THROW("options_name_extractor does not implement this method");
  };

  std::vector<std::shared_ptr<base_option>> get_all_options() override
  {
    THROW("options_name_extractor does not implement this method");
  };

  std::vector<std::shared_ptr<const base_option>> get_all_options() const override
  {
    THROW("options_name_extractor does not implement this method");
  };

  std::shared_ptr<base_option> get_option(const std::string&) override
  {
    THROW("options_name_extractor does not implement this method");
  };

  std::shared_ptr<const base_option> get_option(const std::string&) const override
  {
    THROW("options_name_extractor does not implement this method");
  };

  std::map<std::string, std::vector<option_group_definition>> get_collection_of_options() const override
  {
    THROW("options_name_extractor does not implement this method");
  };

  void insert(const std::string&, const std::string&) override
  {
    THROW("options_name_extractor does not implement this method");
  };

  void replace(const std::string&, const std::string&) override
  {
    THROW("options_name_extractor does not implement this method");
  };

  std::vector<std::string> get_positional_tokens() const override
  {
    THROW("options_name_extractor does not implement this method");
  };
};

struct options_serializer_i
{
  virtual void add(base_option& argument) = 0;
  virtual std::string str() const = 0;
  virtual size_t size() const = 0;
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

bool operator==(const base_option& lhs, const base_option& rhs);
bool operator!=(const base_option& lhs, const base_option& rhs);

inline bool operator==(const base_option& lhs, const base_option& rhs)
{
  return lhs.m_name == rhs.m_name && lhs.m_type_hash == rhs.m_type_hash && lhs.m_help == rhs.m_help &&
      lhs.m_short_name == rhs.m_short_name && lhs.m_keep == rhs.m_keep && lhs.m_necessary == rhs.m_necessary;
}

inline bool operator!=(const base_option& lhs, const base_option& rhs) { return !(lhs == rhs); }

}  // namespace config
}  // namespace VW
