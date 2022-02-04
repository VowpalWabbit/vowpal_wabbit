// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include <memory>
#include <vector>
#include <sstream>
#include <set>
#include <algorithm>
#include <string>
#include <unordered_map>

#include "config/option.h"
#include "config/options.h"

#include "vw_exception.h"
#include "vw_string_view.h"
#include "parse_primitives.h"

namespace VW
{
namespace config
{
bool is_number(const VW::string_view& s)
{
  size_t endidx = 0;
  auto f = parseFloat(s.begin(), endidx, s.end());
  if ((endidx == 0 && !s.empty()) || std::isnan(f)) { return false; }

  return true;
}

bool case_insensitive_equal(VW::string_view a, VW::string_view b)
{
  if (a.length() == b.length())
  {
    return std::equal(b.begin(), b.end(), a.begin(),
        [](unsigned char a, unsigned char b) { return std::tolower(a) == std::tolower(b); });
  }
  return false;
}

bool to_bool(VW::string_view token)
{
  if (case_insensitive_equal(token, "true")) { return true; }
  if (case_insensitive_equal(token, "false")) { return false; }
  THROW("Expected 'true' or 'false', got '" << token << "'");
}

template <typename T>
std::vector<T> flatten_vectors(const std::vector<std::vector<T>>& vec_of_vecs)
{
  std::vector<T> result;
  for (const auto& vec : vec_of_vecs) { result.insert(result.end(), vec.begin(), vec.end()); }
  return result;
}

void check_disagreeing_option_values(
    const std::string& ref_value, const std::string& name, const std::vector<std::string>& final_arguments)
{
  for (auto const& item : final_arguments)
  {
    if (item != ref_value)
    {
      std::stringstream ss;
      ss << "Disagreeing option values for '" << name << "': '" << ref_value << "' vs '" << item << "'";
      THROW_EX(VW::vw_argument_disagreement_exception, ss.str());
    }
  }
}

namespace details
{
struct cli_typed_option_handler : typed_option_visitor
{
  options_cli& m_options;

  cli_typed_option_handler(options_cli& options) : m_options(options) {}

  void visit(typed_option<uint32_t>& option) override { handle_typed_option(option); }
  void visit(typed_option<uint64_t>& option) override { handle_typed_option(option); }
  void visit(typed_option<int64_t>& option) override { handle_typed_option(option); }
  void visit(typed_option<int32_t>& option) override { handle_typed_option(option); }
  void visit(typed_option<bool>& option) override { handle_typed_option(option); }
  void visit(typed_option<float>& option) override { handle_typed_option(option); }
  void visit(typed_option<std::string>& option) override { handle_typed_option(option); }
  void visit(typed_option<std::vector<std::string>>& option) override { handle_typed_option(option); }

  std::vector<std::vector<std::string>> merge_short_and_long_name_lists(base_option& opt)
  {
    std::vector<std::vector<std::string>> result;
    {
      auto it = m_options.m_tokens.find(opt.m_name);
      if (it != m_options.m_tokens.end()) { result.insert(result.end(), it->second.begin(), it->second.end()); }
    }

    if (!opt.m_short_name.empty())
    {
      auto it = m_options.m_tokens.find(opt.m_short_name);
      if (it != m_options.m_tokens.end()) { result.insert(result.end(), it->second.begin(), it->second.end()); }
    }

    return result;
  }

  void move_excess_to_positional(std::vector<std::vector<std::string>>& token_lists)
  {
    for (auto& list : token_lists)
    {
      if (list.size() > 1)
      {
        for (size_t i = 1; i < list.size(); i++) { m_options.m_positional_tokens.push_back(list[i]); }
        list.erase(list.begin() + 1, list.end());
      }
    }
  }

  // 1. bool options. If zero args, set to true. If there are args process them. If there are any >1 sized vectors move
  // them to positional.

  template <typename T>
  void handle_typed_option(typed_option<T>& option)
  {
    if (m_options.m_tokens.find(option.m_name) != m_options.m_tokens.end())
    {
      move_excess_to_positional(m_options.m_tokens[option.m_name]);
    }
    if (m_options.m_tokens.find(option.m_short_name) != m_options.m_tokens.end())
    {
      move_excess_to_positional(m_options.m_tokens[option.m_name]);
    }

    auto all_tokens = merge_short_and_long_name_lists(option);
    // Default case
    if (all_tokens.empty())
    {
      if (option.default_value_supplied()) { option.value(option.default_value(), true); }
    }
    // Handle values
    else
    {
      for (auto& token_list : all_tokens)
      {
        if (token_list.empty()) { THROW("No value supplied for " << option.m_name); }
      }
      // At this point there are 1 or more non-empty token lists.
      auto flattened_tokens = flatten_vectors(all_tokens);

      // Due to the way options get added to the vector, the model options are at the end, and the
      // command-line options are at the front. To allow override from command-line over model file,
      // simply keep the first item.
      std::string token_to_use = flattened_tokens.front();
      if (!option.m_allow_override) { check_disagreeing_option_values(token_to_use, option.m_name, flattened_tokens); }

      m_options.m_supplied_options.insert(option.m_name);
      if (!option.m_short_name.empty()) { m_options.m_supplied_options.insert(option.m_short_name); }

      T value;
      std::stringstream ss;
      ss << token_to_use;
      ss >> value;
      option.value(value, true);
    }
  }

  template <>
  void handle_typed_option<bool>(typed_option<bool>& option)
  {
    if (m_options.m_tokens.find(option.m_name) != m_options.m_tokens.end())
    {
      move_excess_to_positional(m_options.m_tokens[option.m_name]);
    }
    if (m_options.m_tokens.find(option.m_short_name) != m_options.m_tokens.end())
    {
      move_excess_to_positional(m_options.m_tokens[option.m_name]);
    }

    auto all_tokens = merge_short_and_long_name_lists(option);
    if (all_tokens.empty())
    {
      if (option.default_value_supplied()) { option.value(option.default_value(), true); }
    }
    // Handle values
    else
    {
      // Empty token lists for bools are implicit true
      for (auto& token_list : all_tokens)
      {
        if (token_list.empty()) { token_list.push_back("true"); }
      }
      // At this point there are 1 or more non-empty token lists.
      auto flattened_tokens = flatten_vectors(all_tokens);

      // Due to the way options get added to the vector, the model options are at the end, and the
      // command-line options are at the front. To allow override from command-line over model file,
      // simply keep the first item.
      std::string token_to_use = flattened_tokens.front();
      if (!option.m_allow_override) { check_disagreeing_option_values(token_to_use, option.m_name, flattened_tokens); }

      m_options.m_supplied_options.insert(option.m_name);
      if (!option.m_short_name.empty()) { m_options.m_supplied_options.insert(option.m_short_name); }

      option.value(to_bool(token_to_use), true);
    }
  }

  template <>
  void handle_typed_option<std::vector<std::string>>(typed_option<std::vector<std::string>>& option)
  {
    auto all_tokens = merge_short_and_long_name_lists(option);

    // Default case
    if (all_tokens.empty())
    {
      if (option.default_value_supplied()) { option.value(option.default_value(), true); }
    }
    // Handle values
    else
    {
      for (auto& token_list : all_tokens)
      {
        if (token_list.empty()) { THROW("No value supplied for " << option.m_name); }
      }
      // At this point there are 1 or more non-empty token lists.
      auto flattened_tokens = flatten_vectors(all_tokens);

      m_options.m_supplied_options.insert(option.m_name);
      if (!option.m_short_name.empty()) { m_options.m_supplied_options.insert(option.m_short_name); }
      option.value(flattened_tokens, true);
    }
  }
};

}  // namespace details

options_cli::options_cli(int argc, char** argv) : options_cli(std::vector<std::string>(argv + 1, argv + argc)) {}

options_cli::options_cli(const std::vector<std::string>& args) : m_command_line(args)
{
  std::string last_opt = "";
  for (const auto& arg : args)
  {
    if (arg.find("--") == 0)
    {
      auto current_opt = arg.substr(2);
      auto equal_sign_pos = current_opt.find('=');
      if (equal_sign_pos == std::string::npos)
      {
        last_opt = current_opt;
        m_tokens[current_opt].push_back(std::vector<std::string>{});
      }
      else
      {
        auto opt_value = current_opt.substr(equal_sign_pos + 1);
        current_opt = current_opt.substr(0, equal_sign_pos);
        last_opt = current_opt;
        if (opt_value.empty()) { THROW("Argument for " << current_opt << " should follow ="); }
        m_tokens[current_opt].push_back(std::vector<std::string>{});
        m_tokens[current_opt].back().push_back(opt_value);
      }
    }
    else if (arg.find('-') == 0)
    {
      auto num = is_number(arg);
      // This is actually a negative value, treat it as a value.
      if (num)
      {
        // direct value
        if (last_opt.empty()) { m_positional_tokens.push_back(arg); }
        else
        {
          m_tokens[last_opt].back().push_back(arg);
        }
      }
      else
      {
        auto current_opt = arg.substr(1);
        last_opt = current_opt;
        m_tokens[current_opt].push_back(std::vector<std::string>{});
      }
    }
    else
    {
      // direct value
      if (last_opt.empty()) { m_positional_tokens.push_back(arg); }
      else
      {
        m_tokens[last_opt].back().push_back(arg);
      }
    }
  }
}

void options_cli::internal_add_and_parse(const option_group_definition& group)
{
  for (const auto& opt_ptr : group.m_options)
  {
    details::cli_typed_option_handler handler(*this);
    opt_ptr->accept(handler);
    m_defined_options[opt_ptr->m_name].push_back(opt_ptr);
    m_defined_options[opt_ptr->m_short_name].push_back(opt_ptr);
  }

  const auto contains_necessary_options = group.contains_necessary_options();
  const auto is_necessary_enabled = group.check_necessary_enabled(*this);

  // These options are only reachable if necessary was also passed.
  for (const auto& opt_ptr : group.m_options)
  {
    if ((contains_necessary_options && is_necessary_enabled) || !contains_necessary_options)
    {
      m_reachable_options.insert(opt_ptr->m_name);
      m_reachable_options.insert(opt_ptr->m_short_name);
      m_reachable_options.insert("-" + opt_ptr->m_short_name);
    }

    if (contains_necessary_options)
    {
      // We need to convert the unordered set to an ordered one for stable output.
      std::set<std::string> necessary_flags_set(group.m_necessary_flags.begin(), group.m_necessary_flags.end());
      m_dependent_necessary_options[opt_ptr->m_name].push_back(necessary_flags_set);
      m_dependent_necessary_options[opt_ptr->m_short_name].push_back(necessary_flags_set);
      m_dependent_necessary_options["-" + opt_ptr->m_short_name].push_back(necessary_flags_set);
    }
  }
}
bool options_cli::was_supplied(const std::string& key) const { return m_tokens.find(key) != m_tokens.end(); }

void options_cli::check_unregistered(VW::io::logger& logger)
{
  for (auto const& supplied : m_supplied_options)
  {
    if (m_defined_options.count(supplied) == 0)
    {
      THROW_EX(VW::vw_unrecognised_option_exception, "unrecognised option '--" << supplied << "'");
    }
  }

  for (auto const& supplied : m_supplied_options)
  {
    if (m_reachable_options.count(supplied) == 0)
    {
      const auto& dependent_necessary_options = m_dependent_necessary_options.at(supplied);

      auto message = fmt::format(
          "Option '{}' depends on another option (or combination of options) which was not supplied. Possible "
          "combinations of options which would enable this option are:\n",
          supplied);
      for (const auto& group : dependent_necessary_options)
      {
        message += fmt::format("\t{}\n", fmt::join(group, ", "));
      }

      logger.err_warn(message);
    }
  }
}

void options_cli::insert(const std::string& key, const std::string& value) { m_tokens[key].push_back({value}); }

// Note: does not work for vector options.
void options_cli::replace(const std::string& key, const std::string& value)
{
  m_tokens.erase(key);
  m_tokens[key].push_back({value});
}

std::vector<std::string> options_cli::get_positional_tokens() const { return m_positional_tokens; }

}  // namespace config
}  // namespace VW
