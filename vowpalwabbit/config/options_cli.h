// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include <vector>
#include <set>
#include <string>
#include <unordered_map>

#include "config/option.h"
#include "config/options.h"
#include "future_compat.h"

namespace VW
{
namespace config
{
namespace details
{
struct cli_typed_option_handler;
}

struct options_cli : public options_i, typed_option_visitor
{
  options_cli(std::vector<std::string> args);

  void internal_add_and_parse(const option_group_definition& group) override;
  VW_ATTR(nodiscard) bool was_supplied(const std::string& key) const override;
  void check_unregistered(VW::io::logger& logger) override;
  void insert(const std::string& key, const std::string& value) override;
  void replace(const std::string& key, const std::string& value) override;
  VW_ATTR(nodiscard) std::vector<std::string> get_positional_tokens() const override;
  VW_ATTR(nodiscard) const std::set<std::string>& get_supplied_options() const override;
  VW_ATTR(nodiscard) const std::map<std::string, std::vector<std::vector<std::string>>>& get_tokens() const;

  friend struct details::cli_typed_option_handler;

private:
  void process_command_line();

  std::vector<std::string> m_command_line;

  // Key is either short or long name
  std::map<std::string, std::vector<std::vector<std::string>>> m_tokens;
  std::vector<std::string> m_positional_tokens;

  // Contains long and short option names.
  std::set<std::string> m_supplied_options;

  // Key is both short and long option names;
  std::map<std::string, std::vector<std::shared_ptr<base_option>>> m_defined_options;

  std::set<std::string> m_reachable_options;
  std::unordered_map<std::string, std::vector<std::set<std::string>>> m_dependent_necessary_options;
};

}  // namespace config
}  // namespace VW
