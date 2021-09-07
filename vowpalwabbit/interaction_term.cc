// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "interaction_term.h"

#include "constant.h"

namespace INTERACTIONS
{


bool contains_wildcard(const std::vector<namespace_index>& interaction)
{
  return std::find_if(interaction.begin(), interaction.end(),
             [](const namespace_index& term) { return term == wildcard_namespace; }) != interaction.end();
}
}  // namespace INTERACTIONS
