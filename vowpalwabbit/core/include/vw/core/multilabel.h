// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include "vw/core/label_parser.h"
#include "vw/core/multi_ex.h"
#include "vw/core/multiclass.h"
#include "vw/core/v_array.h"
#include "vw/core/vw_fwd.h"

namespace VW
{
class multilabel_label
{
public:
  VW::v_array<uint32_t> label_v;

  void reset_to_default();
  bool is_test() const;
};

class multilabel_prediction
{
public:
  VW::v_array<uint32_t> label_v;
};

extern VW::label_parser multilabel_label_parser_global;
std::string to_string(const VW::multilabel_label& multilabels);
std::string to_string(const VW::multilabel_prediction& multilabels);

namespace model_utils
{
size_t read_model_field(io_buf&, VW::multilabel_label&);
size_t write_model_field(io_buf&, const VW::multilabel_label&, const std::string&, bool);
}  // namespace model_utils

namespace details
{
void update_stats_multilabel(const VW::workspace& all, const VW::example& ec);
void output_example_prediction_multilabel(VW::workspace& all, const VW::example& ec);
void print_update_multilabel(VW::workspace& all, const VW::example& ec);
}  // namespace details

}  // namespace VW

namespace MULTILABEL  // NOLINT
{
using labels VW_DEPRECATED("Renamed to VW::multilabel_labels") = VW::multilabel_label;
}