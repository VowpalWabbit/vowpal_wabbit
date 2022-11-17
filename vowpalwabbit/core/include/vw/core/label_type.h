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
  CB,       // contextual-bandit
  CB_EVAL,  // contextual-bandit evaluation
  CS,       // cost-sensitive
  MULTILABEL,
  MULTICLASS,
  CCB,  // conditional contextual-bandit
  SLATES,
  NOLABEL,
  CONTINUOUS,  // continuous actions
  // clang-format off
  simple VW_DEPRECATED("VW::label_type_t::simple has been renamed to VW::label_type_t::SIMPLE") = SIMPLE, // NOLINT
  cb VW_DEPRECATED("VW::label_type_t::cb has been renamed to VW::label_type_t::CB") = CB, // NOLINT
  cb_eval VW_DEPRECATED("VW::label_type_t::cb_eval has been renamed to VW::label_type_t::CB_EVAL") = CB_EVAL, // NOLINT
  cs VW_DEPRECATED("VW::label_type_t::cs has been renamed to VW::label_type_t::CS") = CS, // NOLINT
  multilabel VW_DEPRECATED("VW::label_type_t::multilabel has been renamed to VW::label_type_t::MULTILABEL") = MULTILABEL, // NOLINT
  multiclass VW_DEPRECATED("VW::label_type_t::multiclass has been renamed to VW::label_type_t::MULTICLASS") = MULTICLASS, // NOLINT
  ccb VW_DEPRECATED("VW::label_type_t::ccb has been renamed to VW::label_type_t::CCB") = CCB, // NOLINT
  slates VW_DEPRECATED("VW::label_type_t::slates has been renamed to VW::label_type_t::SLATES") = SLATES, // NOLINT
  nolabel VW_DEPRECATED("VW::label_type_t::nolabel has been renamed to VW::label_type_t::NOLABEL") = NOLABEL, // NOLINT
  continuous VW_DEPRECATED("VW::label_type_t::continuous has been renamed to VW::label_type_t::CONTINUOUS") = CONTINUOUS // NOLINT
  // clang-format on
};
string_view to_string(VW::label_type_t);
}  // namespace VW

using label_type_t VW_DEPRECATED(
    "Global namespace label_type_t is deprecated. Use VW::label_type_t.") = VW::label_type_t;
