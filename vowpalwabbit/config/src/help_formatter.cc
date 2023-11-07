// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/config/help_formatter.h"

#include "vw/config/option_group_definition.h"
#include "vw/config/options.h"

std::vector<VW::config::option_group_definition> VW::config::remove_disabled_necessary_options(
    options_i& options, const std::vector<option_group_definition>& groups)
{
  std::vector<option_group_definition> result;
  for (const auto& group : groups)
  {
    if ((group.contains_necessary_options() && group.check_necessary_enabled(options)) ||
        !group.contains_necessary_options())
    {
      result.push_back(group);
    }
  }
  return result;
}
