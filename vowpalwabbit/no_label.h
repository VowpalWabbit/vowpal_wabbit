// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once
#include "label_parser.h"

namespace VW
{
struct example;
struct workspace;
}  // namespace VW

namespace no_label
{
using no_label = char;

void return_no_label_example(VW::workspace& all, void*, VW::example& ec);

extern VW::label_parser no_label_parser;

void print_no_label_update(VW::workspace& all, VW::example& ec);
void output_and_account_no_label_example(VW::workspace& all, VW::example& ec);
}  // namespace no_label
