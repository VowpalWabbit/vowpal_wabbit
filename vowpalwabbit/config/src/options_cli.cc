// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/config/options_cli.h"

#include "vw/common/string_view.h"
#include "vw/common/text_utils.h"
#include "vw/common/vw_exception.h"
#include "vw/config/option.h"

#include <fmt/format.h>

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <map>
#include <memory>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace VW::config;

bool is_number(const std::string& str)
{
  if (str.empty()) { return false; }

  char* ptr = nullptr;
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

template <typename T>
void check_disagreeing_option_values(T value, const std::string& name, const std::vector<T>& final_arguments)
{
  for (auto const& item : final_arguments)
  {
    if (item != value)
    {
      std::stringstream ss;
      ss << "Disagreeing option values for '" << name << "': '" << value << "' vs '" << item << "'";
      THROW_EX(VW::vw_argument_disagreement_exception, ss.str());
    }
  }
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
  SCALAR,
  BOOLEAN,
  VECTOR
};

option_type get_option_type(const base_option& option)
{
  if (option.m_type_hash == typeid(bool).hash_code()) { return option_type::BOOLEAN; }
  if (option.m_type_hash == typeid(std::vector<std::string>).hash_code()) { return option_type::VECTOR; }
  return option_type::SCALAR;
}

bool is_terminator(VW::string_view token) { return token == "--"; }

bool is_long_option_like(VW::string_view token) { return token.find("--") == 0 && token.size() > 2; }

bool is_short_option_like(VW::string_view token)
{
  return (token.find('-') == 0 && token.size() > 1 && token[1] != '-' && !is_number(token));
}

bool is_option_like(VW::string_view token)
{
  return is_long_option_like(token) || is_short_option_like(token) || is_terminator(token);
}

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
    case option_type::SCALAR:
      // If we have not already consumed the single token value from an equal sign, consume it now.
      if (current_tokens.empty())
      {
        if (command_line.empty()) { THROW("Expected value for " << opt.m_name << " but found nothing") }
        current_tokens.push_back(command_line.front());
        command_line.pop();
      }
      break;
    case option_type::BOOLEAN:
      if (!current_tokens.empty())
      {
        THROW("Expected no value for " << opt.m_name << " which is a boolean switch but found " << current_tokens.size()
                                       << " values")
      }
      // Booleans do not get to consume any more tokens.
      break;
    case option_type::VECTOR:
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
    /*out*/ std::unordered_map<VW::string_view, std::vector<VW::string_view>>& result)
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

  if (current_opt.empty()) { THROW("Expected long option name after -- but found nothing") }

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
    /*out*/ std::unordered_map<VW::string_view, std::vector<VW::string_view>>& result)
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

template <typename T>
T convert_token_value(const VW::string_view& token)
{
  T result;
  std::stringstream ss(std::string{token});
  ss >> result;
  if (ss.fail() || ss.rdbuf()->in_avail() != 0)
  {
    THROW_EX(VW::vw_argument_invalid_value_exception, "Failed to convert " << token << " to " << typeid(T).name())
  }
  return result;
}

template <>
std::string convert_token_value<std::string>(const VW::string_view& token)
{
  return std::string{token};
}

class cli_typed_option_handler : public typed_option_visitor
{
public:
  std::unordered_map<VW::string_view, std::vector<VW::string_view>>& m_tokens;

  cli_typed_option_handler(std::unordered_map<VW::string_view, std::vector<VW::string_view>>& tokens) : m_tokens(tokens)
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

      std::vector<T> values;
      values.reserve(all_tokens.size());
      for (const auto& token : all_tokens) { values.push_back(convert_token_value<T>(token)); }

      // Due to the way options get added to the vector, the model options are at the end, and the
      // command-line options are at the front. To allow override from command-line over model file,
      // simply keep the first item.
      const auto& value_to_use = values.front();
      if (!option.m_allow_override) { check_disagreeing_option_values(value_to_use, option.m_name, values); }

      option.value(value_to_use, true);
    }
  }

  void handle_typed_option(typed_option<bool>& option)
  {
    auto tokens_it = m_tokens.find(option.m_name);
    bool option_was_supplied = tokens_it != m_tokens.end();

    if (option_was_supplied)
    {
      // This invariant should be maintained by the tokenization code.
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
      // This invariant should be maintained by the tokenization code.
      assert(!all_tokens.empty());
      option.value(std::vector<std::string>{all_tokens.begin(), all_tokens.end()}, true);
    }
    else if (option.default_value_supplied()) { option.value(option.default_value(), true); }
  }
};

// __positional__ contains everything we don't know about
// IMPORTANT holds views into the given command line args
std::unordered_map<VW::string_view, std::vector<VW::string_view>> parse_token_map_with_current_info(
    const std::vector<std::string>& command_line,
    const std::map<std::string, std::shared_ptr<base_option>>& known_options,
    const std::map<char, std::shared_ptr<base_option>>& known_short_options, bool should_handle_terminator)
{
  std::unordered_map<VW::string_view, std::vector<VW::string_view>> m_map;
  std::queue<VW::string_view> tokens;
  for (auto& arg : command_line) { tokens.push(arg); }

  while (!tokens.empty())
  {
    auto token = tokens.front();
    if (is_long_option_like(token)) { consume_long_option(known_options, tokens, m_map); }
    else if (is_short_option_like(token)) { consume_short_option(known_short_options, tokens, m_map); }
    else
    {
      // If we are to handle terminators that means if we hit it then EVERY subsequent token is positional.
      if (should_handle_terminator && is_terminator(token))
      {
        // pop -- itself
        tokens.pop();
        while (!tokens.empty())
        {
          m_map["__POSITIONAL__"].push_back(tokens.front());
          tokens.pop();
        }
      }
      else
      {
        // This is a standard positional argument.
        m_map["__POSITIONAL__"].push_back(token);
        tokens.pop();
      }
    }
  }
  return m_map;
}

options_cli::options_cli(std::vector<std::string> args) : _command_line(std::move(args)) {}

void options_cli::internal_add_and_parse(const option_group_definition& group)
{
  _prog_parsed_token_map = parse_token_map_with_current_info(_command_line, _options, _short_options, false);
  for (const auto& opt_ptr : group.m_options)
  {
    cli_typed_option_handler handler(_prog_parsed_token_map);
    opt_ptr->accept(handler);
  }

  const auto contains_necessary_options = group.contains_necessary_options();
  const auto is_necessary_enabled = group.check_necessary_enabled(*this);

  // These options are only reachable if necessary was also passed.
  for (const auto& opt_ptr : group.m_options)
  {
    if ((contains_necessary_options && is_necessary_enabled) || !contains_necessary_options)
    {
      _reachable_options.insert(opt_ptr->m_name);
      if (!opt_ptr->m_short_name.empty()) { _reachable_options.insert(opt_ptr->m_short_name); }
    }

    if (contains_necessary_options)
    {
      // We need to convert the unordered set to an ordered one for stable output.
      std::set<std::string> necessary_flags_set(group.m_necessary_flags.begin(), group.m_necessary_flags.end());
      _dependent_necessary_options[opt_ptr->m_name].push_back(necessary_flags_set);
      if (!opt_ptr->m_short_name.empty())
      {
        _dependent_necessary_options[opt_ptr->m_short_name].push_back(necessary_flags_set);
      }
    }
  }
}

bool options_cli::was_supplied(const std::string& key) const
{
  // Best check if the token map.
  if (_prog_parsed_token_map.find(key) != _prog_parsed_token_map.end()) { return true; }

  // If not found there, do a fallback check on the command line itself.
  // Short option
  const auto short_key = "-" + key;
  auto short_option_found = std::any_of(_command_line.begin(), _command_line.end(),
      [&short_key](const std::string& arg) { return VW::starts_with(arg, short_key); });
  if (short_option_found) { return true; }

  const auto long_key = "--" + key;
  auto long_option_found = std::any_of(_command_line.begin(), _command_line.end(),
      [&long_key](const std::string& arg)
      {
        // We need to check that the option starts with --key_name, but we also need to ensure that either the whole
        // token matches or we hit an equals sign denoting the end of the option name. If we don't do this --csoaa and
        // --csoaa_ldf would incorrectly match.
        return VW::starts_with(arg, long_key) && ((arg.size() == long_key.size()) || (arg[long_key.size()] == '='));
      });

  return long_option_found;
}

std::vector<std::string> options_cli::check_unregistered()
{
  // Reparse but this time allowing the terminator to be handled so we don't accidentally interpret a positional
  // argument as an unknown option.
  _prog_parsed_token_map = parse_token_map_with_current_info(_command_line, _options, _short_options, true);

  for (auto str : _prog_parsed_token_map["__POSITIONAL__"])
  {
    if (is_option_like(str)) { THROW_EX(VW::vw_unrecognised_option_exception, "unrecognised option '" << str << "'") }
  }

  std::vector<std::string> warnings;

  for (auto const& kv : _prog_parsed_token_map)
  {
    if (kv.first == "__POSITIONAL__") { continue; }
    const auto supplied = std::string{kv.first};
    if (_reachable_options.count(supplied) == 0)
    {
      const auto& dependent_necessary_options = _dependent_necessary_options.at(supplied);

      auto message = fmt::format(
          "Option '{}' depends on another option (or combination of options) which was not supplied. Possible "
          "combinations of options which would enable this option are:\n",
          supplied);
      for (const auto& group : dependent_necessary_options)
      {
        message += fmt::format("\t{}\n", fmt::join(group, ", "));
      }

      warnings.push_back(message);
    }
  }
  return warnings;
}

void options_cli::insert(const std::string& key, const std::string& value)
{
  _command_line.push_back("--" + key);
  if (!value.empty()) { _command_line.push_back(value); }
}

// Note: does not work for vector options.
void options_cli::replace(const std::string& key, const std::string& value)
{
  auto full_key = "--" + key;
  auto it = std::find(_command_line.begin(), _command_line.end(), full_key);

  // Not found, insert instead.
  if (it == _command_line.end())
  {
    insert(key, value);
    return;
  }

  // Check if it is the final option or the next option is not a value.
  if (it + 1 == _command_line.end() || (*(it + 1)).find("--") != std::string::npos)
  {
    THROW(key + " option does not have a value.");
  }

  // Actually replace the value.
  *(it + 1) = value;
}

std::vector<std::string> options_cli::get_positional_tokens() const
{
  // Reparse but this time allowing the terminator to be handled.
  auto parsed_tokens = parse_token_map_with_current_info(_command_line, _options, _short_options, true);

  std::vector<std::string> positional_tokens;
  auto it = parsed_tokens.find("__POSITIONAL__");
  if (it != parsed_tokens.end())
  {
    for (auto const& token : it->second) { positional_tokens.emplace_back(token); }
  }
  return positional_tokens;
}
