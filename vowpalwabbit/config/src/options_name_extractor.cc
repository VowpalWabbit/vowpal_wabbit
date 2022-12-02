// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/config/options_name_extractor.h"

#include "vw/common/vw_exception.h"

using namespace VW::config;

void options_name_extractor::internal_add_and_parse(const option_group_definition& group)
{
  if (!group.contains_necessary_options()) { THROW("reductions must specify at least one .necessary() option"); }

  if (m_added_help_group_names.count(group.m_name) == 0) { m_added_help_group_names.insert(group.m_name); }
  else { THROW("repeated option_group_definition name: " + group.m_name); }

  generated_name.clear();

  for (const auto& opt : group.m_options)
  {
    if (opt->m_necessary)
    {
      if (generated_name.empty()) { generated_name += opt->m_name; }
      else { generated_name += "_" + opt->m_name; }
    }
  }
}

bool options_name_extractor::was_supplied(const std::string&) const { return false; }

std::vector<std::string> options_name_extractor::check_unregistered()
{
  THROW("options_name_extractor does not implement this method");
}

void options_name_extractor::insert(const std::string&, const std::string&)
{
  THROW("options_name_extractor does not implement this method");
}

void options_name_extractor::replace(const std::string&, const std::string&)
{
  THROW("options_name_extractor does not implement this method");
}

std::vector<std::string> options_name_extractor::get_positional_tokens() const
{
  THROW("options_name_extractor does not implement this method");
}
