// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once
#include "label_parser.h"
#include <vector>

struct example;
struct vw;

namespace COST_SENSITIVE
{
struct wclass
{
  float x;
  uint32_t class_index;
  float partial_prediction;  // a partial prediction: new!
  float wap_value;           // used for wap to store values derived from costs
  bool operator==(wclass j) { return class_index == j.class_index; }
};
/* if class_index > 0, then this is a "normal" example
   if class_index == 0, then:
     if x == -FLT_MAX then this is a 'shared' example
     if x > 0 then this is a label feature vector for (size_t)x
*/

struct label
{
  v_array<wclass> costs;
};

void output_example(vw& all, example& ec);
void finish_example(vw& all, example& ec);
template <class T>
void finish_example(vw& all, T&, example& ec)
{
  finish_example(all, ec);
}

extern label_parser cs_label;

void print_update(
    vw& all, bool is_test, example& ec, std::vector<example*>* ec_seq, bool multilabel, uint32_t prediction);
bool ec_is_example_header(example const& ec);  // example headers look like "0:-1" or just "shared"
}  // namespace COST_SENSITIVE
