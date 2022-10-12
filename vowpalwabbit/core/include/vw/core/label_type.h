// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/common/future_compat.h"
#include "vw/common/string_view.h"

#include <cstdint>

namespace VW
{
enum class label_type_t : uint32_t
{
  SIMPLE,
  // contextual-bandit
  CB,
  // contextual-bandit evaluation
  CB_EVAL,
  // cost-sensitive
  CS,
  MULTILABEL,
  MULTICLASS,
  // conditional contextual-bandit
  CCB,
  SLATES,
  NOLABEL,
  CONTINUOUS,  // continuous actions
  // clang-format off
  simple VW_DEPRECATED("label_type_t::simple renamed to prediction_type_t::SIMPLE. Old name will be removed in VW 10.") = SIMPLE, // NOLINT
  cb VW_DEPRECATED("label_type_t::cb renamed to prediction_type_t::CB. Old name will be removed in VW 10.") = CB, // NOLINT
  cb_eval VW_DEPRECATED("label_type_t::cb_eval renamed to prediction_type_t::CB_EVAL. Old name will be removed in VW 10.") = CB_EVAL, // NOLINT
  cs VW_DEPRECATED("label_type_t::cs renamed to prediction_type_t::CS. Old name will be removed in VW 10.") = CS, // NOLINT
  multilabel VW_DEPRECATED("label_type_t::multilabel renamed to prediction_type_t::MULTILABEL. Old name will be removed in VW 10.") = MULTILABEL, // NOLINT
  multiclass VW_DEPRECATED("label_type_t::multiclass renamed to prediction_type_t::MULTICLASS. Old name will be removed in VW 10.") = MULTICLASS, // NOLINT
  ccb VW_DEPRECATED("label_type_t::ccb renamed to prediction_type_t::CCB. Old name will be removed in VW 10.") = CCB, // NOLINT
  slates VW_DEPRECATED("label_type_t::slates renamed to prediction_type_t::SLATES. Old name will be removed in VW 10.") = SLATES, // NOLINT
  nolabel VW_DEPRECATED("label_type_t::nolabel renamed to prediction_type_t::NOLABEL. Old name will be removed in VW 10.") = NOLABEL, // NOLINT
  continuous VW_DEPRECATED("label_type_t::continuous renamed to label_type_t::CONTINUOUS. Old name will be removed in VW 10.") = CONTINUOUS // NOLINT
  // clang-format on
};
string_view to_string(VW::label_type_t);
}  // namespace VW

using label_type_t VW_DEPRECATED(
    "Global namespace label_type_t is deprecated. Use VW::label_type_t.") = VW::label_type_t;
