// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include "vw/core/label_parser.h"
#include "vw/core/multi_ex.h"
#include "vw/core/vw_fwd.h"

#include <cstdint>
#include <vector>

namespace VW
{
/// if class_index > 0, then this is a "normal" example
/// if class_index == 0, then:
///   if x == -FLT_MAX then this is a 'shared' example
///   if x > 0 then this is a label feature vector for (size_t)x
class cs_class
{
public:
  float x;
  uint32_t class_index;
  float partial_prediction;  // a partial prediction: new!
  float wap_value;           // used for wap to store values derived from costs

  cs_class(float x, uint32_t class_index, float partial_prediction, float wap_value)
      : x(x), class_index(class_index), partial_prediction(partial_prediction), wap_value(wap_value)
  {
  }
  cs_class() : x(0.f), class_index(0), partial_prediction(0.f), wap_value(0.f) {}

  bool operator==(const cs_class& j) const { return class_index == j.class_index; }
};
class cs_label
{
public:
  std::vector<cs_class> costs;
};

extern VW::label_parser cs_label_parser_global;

bool is_cs_example_header(const VW::example& ec);
void default_cs_label(cs_label& ld);
namespace details
{
void output_cs_example(VW::workspace& all, const VW::example& ec);
void output_cs_example(
    VW::workspace& all, const VW::example& ec, const cs_label& cs_label, uint32_t multiclass_prediction);
void finish_cs_example(VW::workspace& all, VW::example& ec);
template <class T>
void finish_cs_example(VW::workspace& all, T&, VW::example& ec)
{
  finish_cs_example(all, ec);
}
void print_cs_update(VW::workspace& all, bool is_test, const VW::example& ec, const VW::multi_ex* ec_seq,
    bool multilabel, uint32_t prediction);
}  // namespace details
}  // namespace VW

namespace COST_SENSITIVE  // NOLINT
{
using label VW_DEPRECATED(
    "COST_SENSITIVE::label renamed to VW::cs_label. COST_SENSITIVE::label will be removed in VW 10.") = VW::cs_label;
using wclass VW_DEPRECATED(
    "COST_SENSITIVE::wclass renamed to VW::cs_class. COST_SENSITIVE::wclass will be removed in VW 10.") = VW::cs_class;

VW_DEPRECATED(
    "COST_SENSITIVE::default_label renamed to VW::default_cs_label. COST_SENSITIVE::default_label will be removed in "
    "VW 10.")
inline void default_label(VW::cs_label& ld) { VW::default_cs_label(ld); }
// example headers look like "0:-1" or just "shared"
VW_DEPRECATED(
    "COST_SENSITIVE::ec_is_example_header renamed to VW::is_cs_example_header. COST_SENSITIVE::ec_is_example_header "
    "will be removed in VW 10.")
inline bool ec_is_example_header(VW::example const& ec) { return VW::is_cs_example_header(ec); }
}  // namespace COST_SENSITIVE

namespace VW
{
namespace model_utils
{
size_t read_model_field(io_buf&, cs_class&);
size_t write_model_field(io_buf&, const cs_class&, const std::string&, bool);
size_t read_model_field(io_buf&, cs_label&);
size_t write_model_field(io_buf&, const cs_label&, const std::string&, bool);
}  // namespace model_utils
}  // namespace VW
