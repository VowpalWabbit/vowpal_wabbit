// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "option_group_definition.h"
#include "options.h"
#include "vw/common/future_compat.h"
#include "vw/common/string_view.h"

#include <set>
#include <string>
#include <unordered_map>
#include <vector>

namespace VW
{
namespace config
{
class options_cli : public options_i
{
public:
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
  std::vector<std::string> _command_line;

  // Key is either short or long name
  std::unordered_map<VW::string_view, std::vector<VW::string_view>> _prog_parsed_token_map;

  std::set<std::string> _reachable_options;
  std::unordered_map<std::string, std::vector<std::set<std::string>>> _dependent_necessary_options;
};

}  // namespace config
}  // namespace VW
