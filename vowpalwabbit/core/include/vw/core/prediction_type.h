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
  MULTICLASS_PROBS,  // not in use (technically oaa.cc)
  DECISION_PROBS,
  ACTION_PDF_VALUE,
  ACTIVE_MULTICLASS,
  NOPRED,
  // clang-format off
  scalar VW_DEPRECATED("VW::prediction_type_t::scalar has been renamed to VW::prediction_type_t::SCALAR") = SCALAR, // NOLINT
  scalars VW_DEPRECATED("VW::prediction_type_t::scalars has been renamed to VW::prediction_type_t::SCALARS") = SCALARS, // NOLINT
  action_scores VW_DEPRECATED("VW::prediction_type_t::action_scores has been renamed to VW::prediction_type_t::ACTION_SCORES") = ACTION_SCORES, // NOLINT
  pdf VW_DEPRECATED("VW::prediction_type_t::pdf has been renamed to VW::prediction_type_t::PDF") = PDF, // NOLINT
  action_probs VW_DEPRECATED("VW::prediction_type_t::action_probs has been renamed to VW::prediction_type_t::ACTION_PROBS") = ACTION_PROBS, // NOLINT
  multiclass VW_DEPRECATED("VW::prediction_type_t::multiclass has been renamed to VW::prediction_type_t::MULTICLASS") = MULTICLASS, // NOLINT
  multilabels VW_DEPRECATED("VW::prediction_type_t::multilabels has been renamed to VW::prediction_type_t::MULTILABELS") = MULTILABELS, // NOLINT
  prob VW_DEPRECATED("VW::prediction_type_t::prob has been renamed to VW::prediction_type_t::PROB") = PROB, // NOLINT
  multiclassprobs VW_DEPRECATED("VW::prediction_type_t::multiclassprobs has been renamed to VW::prediction_type_t::MULTICLASS_PROBS") = MULTICLASS_PROBS, // NOLINT
  decision_probs VW_DEPRECATED("VW::prediction_type_t::decision_probs has been renamed to VW::prediction_type_t::DECISION_PROBS") = DECISION_PROBS, // NOLINT
  action_pdf_value VW_DEPRECATED("VW::prediction_type_t::action_pdf_value has been renamed to VW::prediction_type_t::ACTION_PDF_VALUE") = ACTION_PDF_VALUE, // NOLINT
  active_multiclass VW_DEPRECATED("VW::prediction_type_t::active_multiclass has been renamed to VW::prediction_type_t::ACTIVE_MULTICLASS") = ACTIVE_MULTICLASS, // NOLINT
  nopred VW_DEPRECATED("VW::prediction_type_t::nopred has been renamed to VW::prediction_type_t::NOPRED") = NOPRED // NOLINT
};
string_view to_string(prediction_type_t);
}  // namespace VW

using prediction_type_t VW_DEPRECATED(
    "Global namespace prediction_type_t is deprecated. Use VW::label_type_t.") = VW::prediction_type_t;

VW_DEPRECATED(
    "Global namespace to_string(VW::prediction_type_t) is deprecated. Use VW::to_string(VW::prediction_type_t)")
VW::string_view to_string(VW::prediction_type_t prediction_type);
