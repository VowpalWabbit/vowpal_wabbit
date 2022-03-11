// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include <cstddef>

namespace VW
{
struct example;
struct workspace;
}

struct label_data
{
  float label = 0.f;

  label_data();
  label_data(float label);
  void reset_to_default();
};

struct simple_label_reduction_features
{
  float weight;
  float initial;

  simple_label_reduction_features() { reset_to_default(); }
  simple_label_reduction_features(float weight, float initial) : weight(weight), initial(initial) {}
  void reset_to_default() noexcept
  {
    weight = 1.f;
    initial = 0.f;
  }
};

void return_simple_example(VW::workspace& all, void*, VW::example& ec);
bool summarize_holdout_set(VW::workspace& all, size_t& no_win_counter);
void print_update(VW::workspace& all, const VW::example& ec);
void output_and_account_example(VW::workspace& all, const VW::example& ec);
