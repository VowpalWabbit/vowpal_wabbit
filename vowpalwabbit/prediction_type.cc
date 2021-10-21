// Copyright (c) by respective owners including Yahoo!)
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "prediction_type.h"

#include <cassert>

#define CASE(type) \
  case type:       \
    return #type;

VW::string_view VW::to_string(prediction_type_t prediction_type)
{
  using namespace VW;

  switch (prediction_type)
  {
    CASE(prediction_type_t::scalar)
    CASE(prediction_type_t::scalars)
    CASE(prediction_type_t::action_scores)
    CASE(prediction_type_t::pdf)
    CASE(prediction_type_t::action_probs)
    CASE(prediction_type_t::multiclass)
    CASE(prediction_type_t::multilabels)
    CASE(prediction_type_t::prob)
    CASE(prediction_type_t::multiclassprobs)
    CASE(prediction_type_t::decision_probs)
    CASE(prediction_type_t::action_pdf_value)
    CASE(prediction_type_t::active_multiclass)
    CASE(prediction_type_t::no_pred)
  }

  // The above enum is exhaustive and will warn on a new label type being added due to the lack of `default`
  // The following is required by the compiler, otherwise it things control can reach the end of this function without
  // returning.
  assert(false);
  return "unknown prediction type enum";
}

VW::string_view to_string(VW::prediction_type_t prediction_type) { return VW::to_string(prediction_type); }