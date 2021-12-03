// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once
#include "cb.h"

#include "future_compat.h"
#include "cache.h"
#include "io/logger.h"
#include "io_buf.h"
#include "cb_continuous_label.h"
#include <cfloat>

namespace CB
{
template <typename LabelT = CB::label>
void cache_label_additional_fields(const LabelT& ld, io_buf& cache)
{
  cache.write_value(ld.weight);
}

template <typename LabelT = CB::label, typename LabelElmT = cb_class>
void cache_label(const LabelT& ld, io_buf& cache)
{
  cache.write_value(VW::convert(ld.costs.size()));

  for (size_t i = 0; i < ld.costs.size(); i++) { cache.write_value<LabelElmT>(ld.costs[i]); }

  cache_label_additional_fields<LabelT>(ld, cache);
}

template <typename LabelT>
void default_label_additional_fields(LabelT& ld)
{
  ld.weight = 1;
}

template <typename LabelT = CB::label>
void default_label(LabelT& ld)
{
  ld.costs.clear();
  default_label_additional_fields(ld);
}

template <typename LabelElmT = cb_class>
float get_probability(const LabelElmT& elm)
{
  return elm.probability;
}

template <>
inline float get_probability(const VW::cb_continuous::continuous_label_elm& elm)
{
  return elm.pdf_value;
}

template <typename LabelT = CB::label, typename LabelElmT = cb_class>
bool is_test_label(const LabelT& ld)
{
  if (ld.costs.size() == 0) return true;
  for (size_t i = 0; i < ld.costs.size(); i++)
  {
    auto probability = get_probability<LabelElmT>(ld.costs[i]);
    if (FLT_MAX != ld.costs[i].cost && probability > 0.) return false;
  }
  return true;
}

}  // namespace CB
