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
    CASE(prediction_type_t::scalar)             // NOLINT
    CASE(prediction_type_t::scalars)            // NOLINT
    CASE(prediction_type_t::action_scores)      // NOLINT
    CASE(prediction_type_t::pdf)                // NOLINT
    CASE(prediction_type_t::action_probs)       // NOLINT
    CASE(prediction_type_t::multiclass)         // NOLINT
    CASE(prediction_type_t::multilabels)        // NOLINT
    CASE(prediction_type_t::prob)               // NOLINT
    CASE(prediction_type_t::multiclassprobs)    // NOLINT
    CASE(prediction_type_t::decision_probs)     // NOLINT
    CASE(prediction_type_t::action_pdf_value)   // NOLINT
    CASE(prediction_type_t::active_multiclass)  // NOLINT
    CASE(prediction_type_t::nopred)             // NOLINT
  }

  // The above enum is exhaustive and will warn on a new label type being added due to the lack of `default`
  // The following is required by the compiler, otherwise it things control can reach the end of this function without
  // returning.
  assert(false);
  return "unknown prediction type enum";
}

VW::string_view to_string(VW::prediction_type_t prediction_type) { return VW::to_string(prediction_type); }