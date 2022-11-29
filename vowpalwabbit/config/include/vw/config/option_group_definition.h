// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "option.h"
#include "option_builder.h"
#include "vw/common/future_compat.h"

#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

namespace VW
{
namespace config
{
class options_i;

class option_group_definition
{
public:
  // add second parameter for const string short name
  option_group_definition(const std::string& name);

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

  VW_ATTR(nodiscard) bool contains_necessary_options() const;

  // will check if ALL of 'necessary' options were suplied
  VW_ATTR(nodiscard) bool check_necessary_enabled(const options_i& options) const;

  // will check if one_of condition is met for all options
  void check_one_of() const;

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

}  // namespace config
}  // namespace VW
