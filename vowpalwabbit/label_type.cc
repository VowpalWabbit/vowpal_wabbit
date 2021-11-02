// Copyright (c) by respective owners including Yahoo!)
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "label_type.h"

#include <cassert>

#define CASE(type) \
  case type:       \
    return #type;

VW::string_view to_string(VW::label_type_t label_type)
{
  using namespace VW;
  switch (label_type)
  {
    CASE(label_type_t::simple)
    CASE(label_type_t::cb)
    CASE(label_type_t::cb_eval)
    CASE(label_type_t::cs)
    CASE(label_type_t::multilabel)
    CASE(label_type_t::multiclass)
    CASE(label_type_t::ccb)
    CASE(label_type_t::slates)
    CASE(label_type_t::nolabel)
    CASE(label_type_t::continuous)
  }

  // The above enum is exhaustive and will warn on a new label type being added due to the lack of `default`
  // The following is required by the compiler, otherwise it things control can reach the end of this function without
  // returning.
  assert(false);
  return "unknown label type enum";
}