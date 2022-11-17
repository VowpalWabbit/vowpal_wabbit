// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "help_formatter.h"

#include <string>
#include <vector>

namespace VW
{
namespace config
{
class cli_help_formatter : public help_formatter
{
public:
  std::string format_help(const std::vector<option_group_definition>& groups) override;
};

}  // namespace config
}  // namespace VW
