// Copyright (c) by respective owners including Yahoo!)
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/label_type.h"

#include <cassert>

#define CASE(type) \
  case VW::type:   \
    return #type;

VW::string_view VW::to_string(VW::label_type_t label_type)
{
  switch (label_type)
  {
    case label_type_t::SIMPLE: return "simple";
    case label_type_t::CB: return "cb";
    case label_type_t::CB_EVAL: return "cb_eval";
    case label_type_t::CS: return "cs";
    case label_type_t::MULTILABEL: return "multilabel";
    case label_type_t::MULTICLASS: return "multiclass";
    case label_type_t::CCB: return "ccb";
    case label_type_t::SLATES: return "slates";
    case label_type_t::NOLABEL: return "nolabel";
    case label_type_t::CONTINUOUS: return "continuous";
  }

  // The above enum is exhaustive and will warn on a new label type being added due to the lack of `default`
  // The following is required by the compiler, otherwise it things control can reach the end of this function without
  // returning.
  assert(false);
  return "unknown label type enum";
}