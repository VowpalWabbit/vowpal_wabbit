// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/common/future_compat.h"
#include "vw/common/string_view.h"

#include <cstdint>

namespace VW
{
enum class prediction_type_t : uint32_t
{
  SCALAR,
  SCALARS,
  ACTION_SCORES,
  PDF,
  ACTION_PROBS,
  MULTICLASS,
  MULTILABELS,
  PROB,
  MULTICLASSPROBS,  // not in use (technically oaa.cc)
  DECISION_PROBS,
  ACTION_PDF_VALUE,
  ACTIVE_MULTICLASS,
  NOPRED,
  // clang-format off
  scalar VW_DEPRECATED("prediction_type_t::scalar renamed to prediction_type_t::SCALAR. Old name will be removed in VW 10.") = SCALAR, // NOLINT
  scalars VW_DEPRECATED("prediction_type_t::scalars renamed to prediction_type_t::SCALARS. Old name will be removed in VW 10.") = SCALARS, // NOLINT
  action_scores VW_DEPRECATED("prediction_type_t::action_scores renamed to prediction_type_t::ACTION_SCORES. Old name will be removed in VW 10.") = ACTION_SCORES, // NOLINT
  pdf VW_DEPRECATED("prediction_type_t::pdf renamed to prediction_type_t::PDF. Old name will be removed in VW 10.") = PDF, // NOLINT
  action_probs VW_DEPRECATED("prediction_type_t::action_probs renamed to prediction_type_t::ACTION_PROBS. Old name will be removed in VW 10.") = ACTION_PROBS, // NOLINT
  multiclass VW_DEPRECATED("prediction_type_t::multiclass renamed to prediction_type_t::MULTICLASS. Old name will be removed in VW 10.") = MULTICLASS, // NOLINT
  multilabels VW_DEPRECATED("prediction_type_t::multilabels renamed to prediction_type_t::MULTILABELS. Old name will be removed in VW 10.") = MULTILABELS, // NOLINT
  prob VW_DEPRECATED("prediction_type_t::prob renamed to prediction_type_t::PROB. Old name will be removed in VW 10.") = PROB, // NOLINT
  multiclassprobs VW_DEPRECATED("prediction_type_t::multiclassprobs renamed to prediction_type_t::MULTICLASSPROBS. Old name will be removed in VW 10.") = MULTICLASSPROBS,  // not in use (technically oaa.cc) NOLINT
  decision_probs VW_DEPRECATED("prediction_type_t::decision_probs renamed to prediction_type_t::DECISION_PROBS. Old name will be removed in VW 10.") = DECISION_PROBS, // NOLINT
  action_pdf_value VW_DEPRECATED("prediction_type_t::action_pdf_value renamed to prediction_type_t::ACTION_PDF_VALUE. Old name will be removed in VW 10.") = ACTION_PDF_VALUE, // NOLINT
  active_multiclass VW_DEPRECATED("prediction_type_t::active_multiclass renamed to prediction_type_t::ACTIVE_MULTICLASS. Old name will be removed in VW 10.") = ACTIVE_MULTICLASS, // NOLINT
  nopred VW_DEPRECATED("prediction_type_t::active_multiclass renamed to prediction_type_t::ACTIVE_MULTICLASS. Old name will be removed in VW 10.") = NOPRED // NOLINT
  // clang-format on
};
string_view to_string(prediction_type_t);
}  // namespace VW

using prediction_type_t VW_DEPRECATED(
    "Global namespace prediction_type_t is deprecated. Use VW::label_type_t.") = VW::prediction_type_t;

VW_DEPRECATED(
    "Global namespace to_string(VW::prediction_type_t) is deprecated. Use VW::to_string(VW::prediction_type_t)")
VW::string_view to_string(VW::prediction_type_t prediction_type);
