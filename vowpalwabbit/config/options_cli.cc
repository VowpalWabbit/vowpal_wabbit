// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "config/options_cli.h"

#include <algorithm>
#include <cctype>
#include <iterator>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <queue>

#include "config/option.h"
#include "config/options.h"

#include "vw_exception.h"
#include "vw_string_view.h"

using namespace VW::config;

bool is_number(const std::string& str)
{
  if (str.empty()) { return false; }

  char* ptr;
  std::strtof(str.c_str(), &ptr);
  return (*ptr) == '\0';
}

bool is_number(VW::string_view str) { return is_number(std::string{str}); }

template <typename T>
std::vector<T> flatten_vectors(const std::vector<std::vector<T>>& vec_of_vecs)
{
  std::vector<T> result;
  for (const auto& vec : vec_of_vecs) { result.insert(result.end(), vec.begin(), vec.end()); }
  return result;
}

void check_disagreeing_option_values(const VW::string_view& ref_value, const std::string& option_name,
    const std::vector<VW::string_view>& final_arguments)
{
  for (auto const& item : final_arguments)
  {
    if (item != ref_value)
    {
      std::stringstream ss;
      ss << "Disagreeing option values for '" << option_name << "': '" << ref_value << "' vs '" << item << "'";
      THROW_EX(VW::vw_argument_disagreement_exception, ss.str())
    }
  }
}

enum class option_type
{
  scalar,
  boolean,
  vector
};

option_type get_option_type(const base_option& option)
{
  if (option.m_type_hash == typeid(bool).hash_code()) { return option_type::boolean; }
  if (option.m_type_hash == typeid(std::vector<std::string>).hash_code()) { return option_type::vector; }
  return option_type::scalar;
}

bool is_long_option_like(VW::string_view token) { return token.find("--") == 0; }

bool is_short_option_like(VW::string_view token) { return (token.find('-') == 0 && !is_number(token)); }

bool is_option_like(VW::string_view token) { return is_long_option_like(token) || is_short_option_like(token); }

void consume_until_next_option_like(std::queue<VW::string_view>& command_line, std::vector<VW::string_view>& output)
{
  while (!command_line.empty())
  {
    auto token = command_line.front();
    if (is_option_like(token)) { break; }
    command_line.pop();
    output.push_back(token);
  }
}

void consume_tokens(
    const base_option& opt, std::queue<VW::string_view>& command_line, std::vector<VW::string_view>& current_tokens)
{
  const auto type = get_option_type(opt);

  switch (type)
  {
    case option_type::scalar:
      // If we have not already consumed the single token value from an equal sign, consume it now.
      if (current_tokens.empty())
      {
        if (command_line.empty()) { THROW("Expected value for " << opt.m_name << " but found nothing") }
        current_tokens.push_back(command_line.front());
        command_line.pop();
      }
      break;
    case option_type::boolean:
      if (!current_tokens.empty())
      { THROW("Expected no value for " << opt.m_name << " but found " << current_tokens.size() << " values") }
      // Booleans do not get to consume any more tokens.
      break;
    case option_type::vector:
      // If there was no equals token consume the first token unconditionally.
      if (current_tokens.empty())
      {
        if (command_line.empty()) { THROW("Expected value for " << opt.m_name << " but found nothing") }
        current_tokens.push_back(command_line.front());
        command_line.pop();
      }
      consume_until_next_option_like(command_line, current_tokens);
      break;
  }
}

void consume_long_option(const std::map<std::string, std::shared_ptr<base_option>>& known_options,
    /*in-out*/ std::queue<VW::string_view>& command_line,
    /*out*/ std::map<VW::string_view, std::vector<VW::string_view>>& result)
{
  auto current_token = command_line.front();
  auto current_opt = current_token.substr(2);
  const auto equal_sign_pos = current_opt.find('=');
  std::vector<VW::string_view> current_tokens;
  if (equal_sign_pos != std::string::npos)
  {
    auto opt_value = current_opt.substr(equal_sign_pos + 1);
    current_opt = current_opt.substr(0, equal_sign_pos);
    if (opt_value.empty()) { THROW("Argument for " << current_opt << " should follow =") }
    current_tokens.push_back(opt_value);
  }

  // Consume this token.
  command_line.pop();

  // See if there is a known option for this name
  auto known_opt = known_options.find(std::string{current_opt});
  if (known_opt == known_options.end())
  {
    // If there is no known option we are going to just treat it as an unknown option.
    const auto result_name = "__POSITIONAL__";
    result[result_name].push_back(current_token);
    return;
  }

  const auto& option_spec = *known_opt->second;
  consume_tokens(option_spec, command_line, current_tokens);

  // Will create vector if not exists - this is intentional.
  auto& result_tokens = result[option_spec.m_name];
  for (auto& token : current_tokens) { result_tokens.push_back(token); }
}

void consume_short_option(const std::map<char, std::shared_ptr<base_option>>& known_short_options,
    /*in-out*/ std::queue<VW::string_view>& command_line,
    /*out*/ std::map<VW::string_view, std::vector<VW::string_view>>& result)
{
  auto current_token = command_line.front();

  // Check if just dangling -
  if (current_token.size() == 1)
  {
    const auto result_name = "__POSITIONAL__";
    result[result_name].push_back(current_token);
    command_line.pop();
    return;
  }

  auto current_opt = current_token[1];

  std::vector<VW::string_view> current_tokens;
  if (current_token.size() > 2)
  {
    auto opt_value = current_token.substr(2);
    current_tokens.push_back(opt_value);
  }

  // Consume this token.
  command_line.pop();

  // See if there is a known option for this name
  auto known_opt = known_short_options.find(current_opt);
  if (known_opt == known_short_options.end())
  {
    // If there is no known option we are going to just treat it as an unknown option.
    result["__POSITIONAL__"].push_back(current_token);
    return;
  }

  const auto& option_spec = *known_opt->second;
  consume_tokens(option_spec, command_line, current_tokens);

  // Will create vector if not exists - this is intentional.
  auto& result_tokens = result[option_spec.m_name];
  for (auto& token : current_tokens) { result_tokens.push_back(token); }
}

namespace VW
{
namespace config
{
namespace details
{
struct cli_typed_option_handler : typed_option_visitor
{
  options_cli& m_options;
  std::map<VW::string_view, std::vector<VW::string_view>>& m_tokens;

  cli_typed_option_handler(options_cli& options, std::map<VW::string_view, std::vector<VW::string_view>>& tokens)
      : m_options(options), m_tokens(tokens)
  {
  }

  void visit(typed_option<uint32_t>& option) override { handle_typed_option(option); }
  void visit(typed_option<uint64_t>& option) override { handle_typed_option(option); }
  void visit(typed_option<int64_t>& option) override { handle_typed_option(option); }
  void visit(typed_option<int32_t>& option) override { handle_typed_option(option); }
  void visit(typed_option<bool>& option) override { handle_typed_option(option); }
  void visit(typed_option<float>& option) override { handle_typed_option(option); }
  void visit(typed_option<std::string>& option) override { handle_typed_option(option); }
  void visit(typed_option<std::vector<std::string>>& option) override { handle_typed_option(option); }

  template <typename T>
  void handle_typed_option(typed_option<T>& option)
  {
    auto tokens_it = m_tokens.find(option.m_name);
    bool option_was_supplied = tokens_it != m_tokens.end();

    // Default case
    if (!option_was_supplied)
    {
      if (option.default_value_supplied()) { option.value(option.default_value(), true); }
    }
    // Handle values
    else
    {
      const auto& all_tokens = tokens_it->second;

      // Due to the way options get added to the vector, the model options are at the end, and the
      // command-line options are at the front. To allow override from command-line over model file,
      // simply keep the first item.
      const auto& token_to_use = all_tokens.front();
      if (!option.m_allow_override) { check_disagreeing_option_values(token_to_use, option.m_name, all_tokens); }

      // TODO: more robust and helpful value parsing. i.e. detect if a narrowing float conversion occurs
      T value;
      std::stringstream ss;
      ss << token_to_use;
      ss >> value;
      if (ss.fail() || ss.rdbuf()->in_avail() != 0)
      {
        THROW_EX(VW::vw_argument_invalid_value_exception,
          token_to_use << " is an invalid value for option " << option.m_name)
      }
      option.value(value, true);
    }
  }

  void handle_typed_option(typed_option<bool>& option)
  {
    auto tokens_it = m_tokens.find(option.m_name);
    bool option_was_supplied = tokens_it != m_tokens.end();

    if (option_was_supplied)
    {
      // This invariant should be maintianed by the tokenization code.
      assert(tokens_it->second.empty());
      option.value(true, true);
    }
    else
    {
      if (option.default_value_supplied()) { option.value(option.default_value(), true); }
      else
      {
        // Even if there is no explicit default for bools we set it to false.
        option.value(false, true);
      }
    }
  }

  void handle_typed_option(typed_option<std::vector<std::string>>& option)
  {
    auto tokens_it = m_tokens.find(option.m_name);
    bool option_was_supplied = tokens_it != m_tokens.end();

    if (option_was_supplied)
    {
      const auto& all_tokens = tokens_it->second;
      // This invariant should be maintianed by the tokenization code.
      assert(!all_tokens.empty());
      option.value(std::vector<std::string>{all_tokens.begin(), all_tokens.end()}, true);
    }
    else if (option.default_value_supplied())
    {
      option.value(option.default_value(), true);
    }
  }
};

// __positional__ contains everything we don't know about
// IMPORTANT holds views into the given command line args
std::map<VW::string_view, std::vector<VW::string_view>> parse_token_map_with_current_info(
    const std::vector<std::string>& command_line,
    const std::map<std::string, std::shared_ptr<base_option>>& known_options,
    const std::map<char, std::shared_ptr<base_option>>& known_short_options)
{
  std::map<VW::string_view, std::vector<VW::string_view>> m_map;
  std::queue<VW::string_view> tokens;
  for (auto& arg : command_line) { tokens.push(arg); }

  while (!tokens.empty())
  {
    auto token = tokens.front();
    if (is_long_option_like(token)) { consume_long_option(known_options, tokens, m_map); }
    else if (is_short_option_like(token))
    {
      consume_short_option(known_short_options, tokens, m_map);
    }
    else
    {
      // This is a positional argument.
      m_map["__POSITIONAL__"].push_back(token);
      tokens.pop();
    }
  }
  return m_map;
}

}  // namespace details

}  // namespace config
}  // namespace VW

options_cli::options_cli(std::vector<std::string> args) : m_command_line(std::move(args)) {}

void options_cli::internal_add_and_parse(const option_group_definition& group)
{
  m_prog_parsed_token_map = details::parse_token_map_with_current_info(m_command_line, m_options, m_short_options);
  for (const auto& opt_ptr : group.m_options)
  {
    details::cli_typed_option_handler handler(*this, m_prog_parsed_token_map);
    opt_ptr->accept(handler);
  }

  const auto contains_necessary_options = group.contains_necessary_options();
  const auto is_necessary_enabled = group.check_necessary_enabled(*this);

  // These options are only reachable if necessary was also passed.
  for (const auto& opt_ptr : group.m_options)
  {
    if ((contains_necessary_options && is_necessary_enabled) || !contains_necessary_options)
    {
      m_reachable_options.insert(opt_ptr->m_name);
      if (!opt_ptr->m_short_name.empty()) { m_reachable_options.insert(opt_ptr->m_short_name); }
    }

    if (contains_necessary_options)
    {
      // We need to convert the unordered set to an ordered one for stable output.
      std::set<std::string> necessary_flags_set(group.m_necessary_flags.begin(), group.m_necessary_flags.end());
      m_dependent_necessary_options[opt_ptr->m_name].push_back(necessary_flags_set);
      if (!opt_ptr->m_short_name.empty())
      { m_dependent_necessary_options[opt_ptr->m_short_name].push_back(necessary_flags_set); }
    }
  }
}

bool options_cli::was_supplied(const std::string& key) const
{
  // Best check if the token map.
  if (m_prog_parsed_token_map.find(key) != m_prog_parsed_token_map.end()) { return true; }

  // If not found there, do a fallback check on the command line itself.
  auto keys = {std::string("--" + key), std::string("-" + key)};
  return std::find_first_of(std::begin(m_command_line), std::end(m_command_line), std::begin(keys), std::end(keys)) !=
      std::end(m_command_line);
}

void options_cli::check_unregistered(VW::io::logger& logger)
{
  for (auto str : m_prog_parsed_token_map["__POSITIONAL__"])
  {
    if (is_option_like(str)) { THROW_EX(VW::vw_unrecognised_option_exception, "unrecognised option '" << str << "'") }
  }

  for (auto const& kv : m_prog_parsed_token_map)
  {
    const auto supplied = std::string{kv.first};
    if (m_reachable_options.count(supplied) == 0)
    {
      const auto& dependent_necessary_options = m_dependent_necessary_options.at(supplied);

      auto message = fmt::format(
          "Option '{}' depends on another option (or combination of options) which was not supplied. Possible "
          "combinations of options which would enable this option are:\n",
          supplied);
      for (const auto& group : dependent_necessary_options)
      { message += fmt::format("\t{}\n", fmt::join(group, ", ")); }

      logger.err_warn(message);
    }
  }
}

void options_cli::insert(const std::string& key, const std::string& value)
{
  m_command_line.push_back("--" + key);
  if (!value.empty()) { m_command_line.push_back(value); }
}

// Note: does not work for vector options.
void options_cli::replace(const std::string& key, const std::string& value)
{
  auto full_key = "--" + key;
  auto it = std::find(m_command_line.begin(), m_command_line.end(), full_key);

  // Not found, insert instead.
  if (it == m_command_line.end())
  {
    insert(key, value);
    return;
  }

  // Check if it is the final option or the next option is not a value.
  if (it + 1 == m_command_line.end() || (*(it + 1)).find("--") != std::string::npos)
  { THROW(key + " option does not have a value."); }

  // Actually replace the value.
  *(it + 1) = value;
}

std::vector<std::string> options_cli::get_positional_tokens() const
{
  std::vector<std::string> positional_tokens;
  auto it = m_prog_parsed_token_map.find("__POSITIONAL__");
  if (it != m_prog_parsed_token_map.end())
  {
    for (auto const& token : it->second) { positional_tokens.push_back(std::string{token}); }
  }
  return positional_tokens;
}
