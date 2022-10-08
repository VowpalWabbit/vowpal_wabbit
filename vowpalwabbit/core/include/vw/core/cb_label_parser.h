// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/core/cb.h"
#include "vw/core/cb_continuous_label.h"

#include <cfloat>

namespace VW
{
namespace details
{
template <typename LabelT>
void default_cb_label_additional_fields(LabelT& ld)
{
  ld.weight = 1;
}

template <typename LabelT = cb_label>
void default_cb_label(LabelT& ld)
{
  ld.costs.clear();
  default_cb_label_additional_fields(ld);
}

template <typename LabelElmT = cb_class>
float get_cb_probability(const LabelElmT& elm)
{
  return elm.probability;
}

template <>
inline float get_cb_probability(const VW::cb_continuous::continuous_label_elm& elm)
{
  return elm.pdf_value;
}

template <typename LabelT = cb_label, typename LabelElmT = cb_class>
bool is_test_cb_label(const LabelT& ld)
{
  if (ld.costs.size() == 0) { return true; }
  for (size_t i = 0; i < ld.costs.size(); i++)
  {
    auto probability = get_cb_probability<LabelElmT>(ld.costs[i]);
    if (FLT_MAX != ld.costs[i].cost && probability > 0.) { return false; }
  }
  return true;
}
}  // namespace details

}  // namespace VW
