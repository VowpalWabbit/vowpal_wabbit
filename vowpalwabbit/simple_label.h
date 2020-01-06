// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once
#include "label_parser.h"

#include <cfloat>

struct example;
struct vw;

struct label_data
{
  float label;
  float weight;
  float initial;

  label_data() : label(FLT_MAX), weight(0.f), initial(0.f) {}
  label_data(float label, float weight, float initial) : label(label), weight(weight), initial(initial) {}
};

void return_simple_example(vw& all, new_polylabel&, example& ec);
void return_simple_example_explicit(vw& all, example& ec);

extern label_parser simple_label;

bool summarize_holdout_set(vw& all, size_t& no_win_counter);
void print_update(vw& all, example& ec);
void output_and_account_example(vw& all, example& ec);
