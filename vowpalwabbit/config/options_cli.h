// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include <vector>
#include <set>
#include <string>
#include <unordered_map>
#include <iostream>

#include "config/option.h"
#include "config/options.h"
#include "future_compat.h"
#include "vw_string_view.h"

namespace VW
{
namespace config
{
namespace details
{
struct cli_typed_option_handler;
}  // namespace details

struct options_cli : public options_i
{
  options_cli(std::vector<std::string> args);

  void internal_add_and_parse(const option_group_definition& group) override;
  VW_ATTR(nodiscard) bool was_supplied(const std::string& key) const override;
  void check_unregistered(VW::io::logger& logger) override;
  void insert(const std::string& key, const std::string& value) override;
  void replace(const std::string& key, const std::string& value) override;
  VW_ATTR(nodiscard) std::vector<std::string> get_positional_tokens() const override;

  friend struct details::cli_typed_option_handler;

private:
  std::vector<std::string> m_command_line;

  // Key is either short or long name
  std::unordered_map<VW::string_view, std::vector<VW::string_view>> m_prog_parsed_token_map;

  std::set<std::string> m_reachable_options;
  std::unordered_map<std::string, std::vector<std::set<std::string>>> m_dependent_necessary_options;
};

}  // namespace config
}  // namespace VW
