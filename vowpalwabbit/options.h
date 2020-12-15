// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include <string>
#include <utility>
#include <vector>
#include <typeinfo>
#include <memory>
#include <unordered_set>
#include <sstream>
#include <type_traits>

#include "options_types.h"
#include "vw_exception.h"

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
  bool m_necessary = false;
  bool m_allow_override = false;

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

  bool default_value_supplied() const { return m_default_value.get() != nullptr; }

  T default_value() const { return m_default_value ? *m_default_value : T(); }

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

  typed_option& necessary(bool necessary = true)
  {
    m_necessary = necessary;
    return *this;
  }

  typed_option& allow_override(bool allow_override = true)
  {
    static_assert(is_scalar_option_type<T>::value, "allow_override can only apply to scalar option types.");
    m_allow_override = allow_override;
    return *this;
  }

  bool value_supplied() const { return m_value.get() != nullptr; }

  typed_option& value(T value)
  {
    m_value = std::make_shared<T>(value);
    return *this;
  }

  T value() const { return m_value ? *m_value : T(); }

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

struct option_group_definition;

struct options_i
{
  virtual void add_and_parse(const option_group_definition& group) = 0;
  virtual bool add_parse_and_check_necessary(const option_group_definition& group) = 0;
  virtual bool was_supplied(const std::string& key) const = 0;
  virtual std::string help() const = 0;

  virtual std::vector<std::shared_ptr<base_option>> get_all_options() = 0;
  virtual std::vector<std::shared_ptr<const base_option>> get_all_options() const = 0;
  virtual std::shared_ptr<base_option> get_option(const std::string& key) = 0;
  virtual std::shared_ptr<const base_option> get_option(const std::string& key) const = 0;

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

  template <typename T>
  struct is_vector
  {
    static const bool value = false;
  };

  template <typename T, typename A>
  struct is_vector<std::vector<T, A>>
  {
    static const bool value = true;
  };

  // Check if option values exist and match.
  // Add if it does not exist.
  template <typename T>
  bool insert_arguments(const std::string& name, T expected_val)
  {
    static_assert(!is_vector<T>::value, "insert_arguments does not support vectors");

    if (was_supplied(name))
    {
      T found_val = get_typed_option<T>(name).value();
      if (found_val != expected_val) { return false; }
    }
    else
    {
      std::stringstream ss;
      ss << expected_val;
      insert(name, ss.str());
    }
    return true;
  }

  // Will throw if any options were supplied that do not having a matching argument specification.
  virtual void check_unregistered() = 0;

  virtual ~options_i() = default;
};

struct option_group_definition
{
  // add second parameter for const string short name
  option_group_definition(const std::string& name) : m_name(name) { m_importance = 0; }

  template <typename T>
  option_group_definition& add(T&& op)
  {
    m_options.push_back(std::make_shared<typename std::decay<T>::type>(op));
    return *this;
  }

  template <typename T>
  option_group_definition& add(typed_option<T>& op)
  {
    m_options.push_back(std::make_shared<typename std::decay<typed_option<T>>::type>(op));
    if (op.m_necessary) { m_necessary_flags.insert(op.m_name); }

    return *this;
  }

  // will check if ALL of 'necessary' options were suplied
  bool check_necessary_enabled(const options_i& options) const
  {
    if (m_necessary_flags.size() == 0) return false;

    bool check_if_all_necessary_enabled = true;

    for (const auto& elem : m_necessary_flags) { check_if_all_necessary_enabled &= options.was_supplied(elem); }

    return check_if_all_necessary_enabled;
  }

  template <typename T>
  option_group_definition& operator()(T&& op)
  {
    add(std::forward<T>(op));
    return *this;
  }

  // the smaller the number the more importance
  // used only to order the --help page
  option_group_definition& importance(int imp)
  {
    m_importance = imp;
    return *this;
  }

  std::string m_name;
  std::unordered_set<std::string> m_necessary_flags;
  std::vector<std::shared_ptr<base_option>> m_options;
  int m_importance;
};

struct options_name_extractor : options_i
{
  std::string generated_name;

  void add_and_parse(const option_group_definition&) override
  {
    THROW("you should use add_parse_and_check_necessary() inside a reduction setup");
  };

  bool add_parse_and_check_necessary(const option_group_definition& group) override
  {
    if (group.m_necessary_flags.empty()) { THROW("reductions must specify at least one .necessary() option"); }

    generated_name.clear();

    for (auto opt : group.m_options)
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

  std::string help() const override { THROW("options_name_extractor does not implement this method"); };

  void check_unregistered() override { THROW("options_name_extractor does not implement this method"); };

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
