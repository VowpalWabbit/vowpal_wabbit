// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/config/option_group_definition.h"

#include "vw/config/options.h"

using namespace VW::config;

option_group_definition::option_group_definition(const std::string& name) : m_name(name + " Options") {}

bool option_group_definition::contains_necessary_options() const { return !m_necessary_flags.empty(); }

bool option_group_definition::check_necessary_enabled(const options_i& options) const
{
  if (m_necessary_flags.empty()) { return false; }

  bool check_if_all_necessary_enabled = true;

  for (const auto& elem : m_necessary_flags) { check_if_all_necessary_enabled &= options.was_supplied(elem); }

  return check_if_all_necessary_enabled;
}

void option_group_definition::check_one_of() const
{
  for (const auto& option : m_options)
  {
    if (!option->m_one_of_err.empty()) { THROW(option->m_one_of_err); }
  }
}
