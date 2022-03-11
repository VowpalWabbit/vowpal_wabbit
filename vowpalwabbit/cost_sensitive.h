// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include <cstdint>
#include <vector>

#include "io_buf.h"
#include "label_parser.h"
#include "v_array.h"

namespace VW
{
struct example;
struct workspace;
}

namespace COST_SENSITIVE
{
struct wclass
{
  float x;
  uint32_t class_index;
  float partial_prediction;  // a partial prediction: new!
  float wap_value;           // used for wap to store values derived from costs

  wclass(float x, uint32_t class_index, float partial_prediction, float wap_value)
      : x(x), class_index(class_index), partial_prediction(partial_prediction), wap_value(wap_value)
  {
  }
  wclass() : x(0.f), class_index(0), partial_prediction(0.f), wap_value(0.f) {}

  bool operator==(wclass j) { return class_index == j.class_index; }
};
/* if class_index > 0, then this is a "normal" example
   if class_index == 0, then:
     if x == -FLT_MAX then this is a 'shared' example
     if x > 0 then this is a label feature vector for (size_t)x
*/

struct label
{
  std::vector<wclass> costs;
};

void output_example(VW::workspace& all, const VW::example& ec);
void output_example(
    VW::workspace& all, const VW::example& ec, const COST_SENSITIVE::label& cs_label, uint32_t multiclass_prediction);
void finish_example(VW::workspace& all, VW::example& ec);
template <class T>
void finish_example(VW::workspace& all, T&, VW::example& ec)
{
  COST_SENSITIVE::finish_example(all, ec);
}

void default_label(label& ld);
extern VW::label_parser cs_label;

void print_update(VW::workspace& all, bool is_test, const VW::example& ec, const std::vector<VW::example*>* ec_seq,
    bool multilabel, uint32_t prediction);
bool ec_is_example_header(VW::example const& ec);  // example headers look like "0:-1" or just "shared"
}  // namespace COST_SENSITIVE

namespace VW
{
namespace model_utils
{
size_t read_model_field(io_buf&, COST_SENSITIVE::wclass&);
size_t write_model_field(io_buf&, const COST_SENSITIVE::wclass&, const std::string&, bool);
size_t read_model_field(io_buf&, COST_SENSITIVE::label&);
size_t write_model_field(io_buf&, const COST_SENSITIVE::label&, const std::string&, bool);
}  // namespace model_utils
}  // namespace VW
