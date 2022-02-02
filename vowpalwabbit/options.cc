// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "options.h"

using namespace VW::config;

void options_i::add_and_parse(const option_group_definition& group)
{
  internal_add_and_parse(group);
  group.check_one_of();
}

bool options_i::add_parse_and_check_necessary(const option_group_definition& group)
{
  internal_add_and_parse(group);
  return group.check_necessary_enabled(*this) && group.check_one_of();
}