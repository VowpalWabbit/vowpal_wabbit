// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/config/options.h"

#include "vw/config/option.h"
#include "vw/config/option_group_definition.h"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <stdexcept>
#include <utility>

using namespace VW::config;

std::vector<std::shared_ptr<base_option>> options_i::get_all_options()
{
  std::vector<std::shared_ptr<base_option>> output_values;

  std::transform(_options.begin(), _options.end(), std::back_inserter(output_values),
      [](std::pair<const std::string, std::shared_ptr<base_option>>& kv) { return kv.second; });

  return output_values;
}

std::vector<std::shared_ptr<const base_option>> VW::config::options_i::get_all_options() const
{
  std::vector<std::shared_ptr<const base_option>> output_values;
  output_values.reserve(_options.size());
  for (const auto& kv : _options) { output_values.push_back(kv.second); }
  return output_values;
}

// This function is called by both the const and non-const version. The const version will implicitly upgrade the
// shared_ptr to const
std::shared_ptr<base_option> internal_get_option(
    const std::string& key, const std::map<std::string, std::shared_ptr<VW::config::base_option>>& options)
{
  auto it = options.find(key);
  if (it != options.end()) { return it->second; }

  throw std::out_of_range(key + " was not found.");
}

std::shared_ptr<base_option> VW::config::options_i::get_option(const std::string& key)
{
  return internal_get_option(key, _options);
}

std::shared_ptr<const base_option> VW::config::options_i::get_option(const std::string& key) const
{
  // shared_ptr can implicitly upgrade to const from non-const
  return internal_get_option(key, _options);
}

void options_i::add_and_parse(const option_group_definition& group)
{
  // Add known options before parsing so impl can make use of them.
  _option_group_definitions.push_back(group);
  _option_group_dic[_current_reduction_tint].push_back(group);
  for (const auto& option : group.m_options)
  {
    // The last definition is kept. There was a bug where using .insert at a later pointer changed the command line but
    // the previously defined option's default value was serialized into the model. This resolves that state info.
    _options[option->m_name] = option;
    if (!option->m_short_name.empty())
    {
      assert(option->m_short_name.size() == 1);
      _short_options[option->m_short_name[0]] = option;
    }
  }

  internal_add_and_parse(group);

  group.check_one_of();
}

bool options_i::add_parse_and_check_necessary(const option_group_definition& group)
{
  // Add known options before parsing so impl can make use of them.
  _option_group_definitions.push_back(group);
  _option_group_dic[_current_reduction_tint].push_back(group);
  for (const auto& option : group.m_options)
  {
    // The last definition is kept. There was a bug where using .insert at a later pointer changed the command line but
    // the previously defined option's default value was serialized into the model. This resolves that state info.
    _options[option->m_name] = option;
    if (!option->m_short_name.empty())
    {
      assert(option->m_short_name.size() == 1);
      _short_options[option->m_short_name[0]] = option;
    }
  }

  internal_add_and_parse(group);

  auto necessary_enabled = group.check_necessary_enabled(*this);
  if (necessary_enabled) { group.check_one_of(); }

  return necessary_enabled;
}

void options_i::tint(const std::string& reduction_name) { _current_reduction_tint = reduction_name; }
void options_i::reset_tint() { _current_reduction_tint = DEFAULT_TINT; }

std::map<std::string, std::vector<option_group_definition>> options_i::get_collection_of_options() const
{
  return _option_group_dic;
}
const std::vector<option_group_definition>& options_i::get_all_option_group_definitions() const
{
  return _option_group_definitions;
}
