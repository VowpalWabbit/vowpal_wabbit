// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "option.h"

#include <memory>
#include <set>
#include <string>
#include <type_traits>
#include <utility>

namespace VW
{
namespace config
{
namespace details
{
template <typename T, typename _ = void>
struct is_vector : std::false_type
{
};

template <typename T>
struct is_vector<T,
    typename std::enable_if<std::is_same<typename std::decay<T>::type,
        std::vector<typename std::decay<T>::type::value_type, typename std::decay<T>::type::allocator_type>>::value>::
        type> : std::true_type
{
};

}  // namespace details

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
    if (short_name.size() != 1) { THROW("short_name must be a single character but got: " << short_name); }
    m_option_obj.m_short_name = short_name;
    return *this;
  }

  option_builder& short_name(char short_name)
  {
    m_option_obj.m_short_name = std::string(1, short_name);
    return *this;
  }

  option_builder& help(const std::string& help)
  {
    m_option_obj.m_help = help;
    return *this;
  }

  /// Hides the option from help output.
  option_builder& hidden(bool hidden = true)
  {
    m_option_obj.m_hidden_from_help = hidden;
    return *this;
  }

  /// Marks this as an experimental option.
  option_builder& experimental(bool experimental = true)
  {
    m_option_obj.m_experimental = experimental;
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
    if (details::is_vector<typename T::value_type>::value)
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

template <typename T>
option_builder<typed_option_with_location<T>> make_option(const std::string& name, T& location)
{
  return typed_option_with_location<T>(name, location);
}

}  // namespace config
}  // namespace VW
