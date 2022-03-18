// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "config/option_group_definition.h"
#include "config/options.h"
#include "future_compat.h"
#include "vw_string_view.h"

namespace VW
{
namespace config
{
struct options_cli : public options_i
{
  options_cli(std::vector<std::string> args);

  void internal_add_and_parse(const option_group_definition& group) override;
  VW_ATTR(nodiscard) bool was_supplied(const std::string& key) const override;
  /**
   * @brief Check for unregistered options and validate input. Throws if there
   * is an error. Returns a vector of warning strings if there are warnings produced.
   */
  VW_ATTR(nodiscard) std::vector<std::string> check_unregistered() override;
  void insert(const std::string& key, const std::string& value) override;
  void replace(const std::string& key, const std::string& value) override;
  VW_ATTR(nodiscard) std::vector<std::string> get_positional_tokens() const override;

private:
  std::vector<std::string> m_command_line;

  // Key is either short or long name
  std::unordered_map<VW::string_view, std::vector<VW::string_view>> m_prog_parsed_token_map;

  std::set<std::string> m_reachable_options;
  std::unordered_map<std::string, std::vector<std::set<std::string>>> m_dependent_necessary_options;
};

}  // namespace config
}  // namespace VW
