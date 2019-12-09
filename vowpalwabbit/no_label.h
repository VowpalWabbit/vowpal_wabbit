// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once
#include "label_parser.h"

struct example;
struct vw;

namespace no_label
{
typedef char no_label;

void return_no_label_example(vw& all, void*, example& ec);

extern label_parser no_label_parser;

void print_no_label_update(vw& all, example& ec);
void output_and_account_no_label_example(vw& all, example& ec);
}  // namespace no_label
