// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include "vw/common/future_compat.h"
#include "vw/core/feature_group.h"
#include "vw/core/vw_fwd.h"

#include <cstddef>

namespace VW
{
class simple_label
{
public:
  float label = 0.f;

  simple_label();
  simple_label(float label);
  void reset_to_default();
};

inline bool operator==(const simple_label& lhs, const simple_label& rhs) { return lhs.label == rhs.label; }
inline bool operator!=(const simple_label& lhs, const simple_label& rhs) { return !(lhs == rhs); }

class simple_label_reduction_features
{
public:
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

namespace details
{
void return_simple_example(VW::workspace& all, void*, VW::example& ec);
bool summarize_holdout_set(VW::workspace& all, size_t& no_win_counter);
void print_update(VW::workspace& all, const VW::example& ec);
void output_and_account_example(VW::workspace& all, const VW::example& ec);
}  // namespace details
}  // namespace VW

using label_data VW_DEPRECATED(
    "label_data renamed to VW::simple_label. label_data will be removed in VW 10.") = VW::simple_label;
using simple_label_reduction_features VW_DEPRECATED(
    "simple_label_reduction_features renamed to VW::simple_label_reduction_features. simple_label_reduction_features "
    "will be removed in VW 10.") = VW::simple_label_reduction_features;
