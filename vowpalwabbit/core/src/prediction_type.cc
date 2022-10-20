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
    CASE(prediction_type_t::SCALAR)
    CASE(prediction_type_t::SCALARS)
    CASE(prediction_type_t::ACTION_SCORES)
    CASE(prediction_type_t::PDF)
    CASE(prediction_type_t::ACTION_PROBS)
    CASE(prediction_type_t::MULTICLASS)
    CASE(prediction_type_t::MULTILABELS)
    CASE(prediction_type_t::PROB)
    CASE(prediction_type_t::MULTICLASS_PROBS)
    CASE(prediction_type_t::DECISION_PROBS)
    CASE(prediction_type_t::ACTION_PDF_VALUE)
    CASE(prediction_type_t::ACTIVE_MULTICLASS)
    CASE(prediction_type_t::NOPRED)
  }

  // The above enum is exhaustive and will warn on a new label type being added due to the lack of `default`
  // The following is required by the compiler, otherwise it things control can reach the end of this function without
  // returning.
  assert(false);
  return "unknown prediction type enum";
}

VW::string_view to_string(VW::prediction_type_t prediction_type) { return VW::to_string(prediction_type); }