// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once
#include "label_parser.h"

struct example;
struct vw;

struct label_data
{
  float label;
  float weight;
  float initial;
};

void return_simple_example(vw& all, void*, example& ec);

extern label_parser simple_label;

bool summarize_holdout_set(vw& all, size_t& no_win_counter);
void print_update(vw& all, example& ec);
void output_and_account_example(vw& all, example& ec);
