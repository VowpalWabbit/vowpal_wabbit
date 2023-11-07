// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include "vw/core/label_parser.h"
#include "vw/core/vw_fwd.h"

namespace VW
{
class multiclass_label
{
public:
  uint32_t label;
  float weight;

  multiclass_label();
  multiclass_label(uint32_t label, float weight);
  void reset_to_default();

  bool is_labeled() const { return label != static_cast<uint32_t>(-1); }
};

extern VW::label_parser multiclass_label_parser_global;

bool test_multiclass_label(const multiclass_label& ld);

namespace details
{
void print_multiclass_update_with_probability(VW::workspace& all, const VW::example& ec, uint32_t prediction);
void print_multiclass_update_with_score(VW::workspace& all, const VW::example& ec, uint32_t prediction);
void finish_multiclass_example(VW::workspace& all, VW::example& ec, bool update_loss);

template <class T>
void finish_multiclass_example(VW::workspace& all, T&, VW::example& ec)
{
  finish_multiclass_example(all, ec, true);
}
template <class T>
void finish_multiclass_example_without_loss(VW::workspace& all, T&, VW::example& ec)
{
  finish_multiclass_example(all, ec, false);
}

void update_stats_multiclass_label(
    const VW::workspace& all, shared_data& sd, const VW::example& ec, VW::io::logger& logger);
void output_example_prediction_multiclass_label(VW::workspace& all, const VW::example& ec, VW::io::logger& logger);
void print_update_multiclass_label(VW::workspace& all, shared_data& sd, const VW::example& ec, VW::io::logger& logger);

template <typename UnusedDataT>
void update_stats_multiclass_label(const VW::workspace& all, shared_data& sd, const UnusedDataT& /* unused */,
    const VW::example& ec, VW::io::logger& logger)
{
  update_stats_multiclass_label(all, sd, ec, logger);
}
template <typename UnusedDataT>
void output_example_prediction_multiclass_label(
    VW::workspace& all, const UnusedDataT& /* unused */, const VW::example& ec, VW::io::logger& logger)
{
  output_example_prediction_multiclass_label(all, ec, logger);
}
template <typename UnusedDataT>
void print_update_multiclass_label(
    VW::workspace& all, shared_data& sd, const UnusedDataT& /* unused */, const VW::example& ec, VW::io::logger& logger)
{
  print_update_multiclass_label(all, sd, ec, logger);
}

}  // namespace details
namespace model_utils
{
size_t read_model_field(io_buf&, VW::multiclass_label&);
size_t write_model_field(io_buf&, const VW::multiclass_label&, const std::string&, bool);
}  // namespace model_utils
}  // namespace VW

namespace MULTICLASS  // NOLINT
{
using label_t VW_DEPRECATED(
    "MULTICLASS::label_t renamed to VW::multiclass_label. MULTICLASS::label_t will be removed in VW 10.") =
    VW::multiclass_label;
VW_DEPRECATED(
    "MULTICLASS::test_label renamed to VW::test_multiclass_label. MULTICLASS::test_label will be removed in VW 10.")
inline bool test_label(const VW::multiclass_label& ld) { return VW::test_multiclass_label(ld); }
}  // namespace MULTICLASS
