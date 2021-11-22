// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw_string_view.h"

#include <cstdint>

namespace VW
{
enum class prediction_type_t : uint32_t
{
  scalar,
  scalars,
  action_scores,
  pdf,
  action_probs,
  multiclass,
  multilabels,
  prob,
  multiclassprobs,
  decision_probs,
  action_pdf_value,
  active_multiclass,
  nopred
};
string_view to_string(prediction_type_t);
}  // namespace VW

using prediction_type_t VW_DEPRECATED(
    "Global namespace prediction_type_t is deprecated. Use VW::label_type_t.") = VW::prediction_type_t;

VW_DEPRECATED(
    "Global namespace to_string(VW::prediction_type_t) is deprecated. Use VW::to_string(VW::prediction_type_t)")
VW::string_view to_string(VW::prediction_type_t prediction_type);
