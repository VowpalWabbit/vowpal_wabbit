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
  // only used for serialization and parsing.  example.weight is used for
  // computation
  // DeSerialized/Parsed values are copied into example in VW::setup_example()
  float serialized_weight = 0.f;
  // Only used for serialization and parsing.  example.initial is used for
  // computation
  // DeSerialized/Parsed values are copied into example in VW::setup_example()
  float serialized_initial = 0.f;

  label_data();
  label_data(float label, float weight, float initial);
  void reset_to_default();
};

void return_simple_example(vw& all, void*, example& ec);

extern label_parser simple_label_parser;

bool summarize_holdout_set(vw& all, size_t& no_win_counter);
void print_update(vw& all, example& ec);
void output_and_account_example(vw& all, example& ec);

namespace VW
{
constexpr float UNUSED_0 = 0.f;  // constant to signal initializing unused member
constexpr float UNUSED_1 = 1.f;  // constant to signal initializing unused member
}  // namespace VW
