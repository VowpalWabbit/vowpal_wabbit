// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include <string>
#include <vector>

namespace VW
{
namespace config
{
class option_group_definition;
class options_i;

std::vector<option_group_definition> remove_disabled_necessary_options(
    options_i& options, const std::vector<option_group_definition>& groups);

class help_formatter
{
public:
  virtual std::string format_help(const std::vector<option_group_definition>& groups) = 0;
  virtual ~help_formatter() = default;
};

}  // namespace config
}  // namespace VW
