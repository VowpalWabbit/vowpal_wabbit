// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once
#include "label_parser.h"

struct example;
struct vw;

namespace nolabel
{
using nolabel = char;

void return_nolabel_example(vw& all, void*, example& ec);

extern label_parser nolabel_parser;

void print_nolabel_update(vw& all, example& ec);
void output_and_account_nolabel_example(vw& all, example& ec);
}  // namespace nolabel
