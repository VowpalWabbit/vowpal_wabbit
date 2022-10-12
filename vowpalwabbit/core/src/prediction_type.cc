// Copyright (c) by respective owners including Yahoo!)
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/prediction_type.h"

#include <cassert>

#define CASE(type) \
  case type:       \
    return #type;

VW::string_view VW::to_string(prediction_type_t prediction_type)
{
  using namespace VW;

  switch (prediction_type)
  {
    case prediction_type_t::SCALAR:
      return "scalar";
    case prediction_type_t::SCALARS:
      return "scalars";
    case prediction_type_t::ACTION_SCORES:
      return "action_scores";
    case prediction_type_t::PDF:
      return "pdf";
    case prediction_type_t::ACTION_PROBS:
      return "action_probs";
    case prediction_type_t::MULTICLASS:
      return "multiclass";
    case prediction_type_t::MULTILABELS:
      return "multilabels";
    case prediction_type_t::PROB:
      return "prob";
    case prediction_type_t::MULTICLASSPROBS:
      return "multiclassprobs";
    case prediction_type_t::DECISION_PROBS:
      return "decision_probs";
    case prediction_type_t::ACTION_PDF_VALUE:
      return "action_pdf_value";
    case prediction_type_t::ACTIVE_MULTICLASS:
      return "active_multiclass";
    case prediction_type_t::NOPRED:
      return "nopred";
  }

  // The above enum is exhaustive and will warn on a new label type being added due to the lack of `default`
  // The following is required by the compiler, otherwise it things control can reach the end of this function without
  // returning.
  assert(false);
  return "unknown prediction type enum";
}

VW::string_view to_string(VW::prediction_type_t prediction_type) { return VW::to_string(prediction_type); }