// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once
#include "label_parser.h"
#include <cmath>

struct example;
struct vw;

struct label_data
{
  float label = 0.f;

  label_data();
  label_data(float label);
  void reset_to_default();
};

struct simple_label_reduction_features
{
  float weight = 1.f;
  float initial = 0.f;
  void reset_to_default();
};

void return_simple_example(vw& all, void*, example& ec);

extern label_parser simple_label_parser;

bool summarize_holdout_set(vw& all, size_t& no_win_counter);
void print_update(vw& all, example& ec);
void output_and_account_example(vw& all, example& ec);
