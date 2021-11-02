// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once
#include "cb.h"

#include "future_compat.h"
#include "io/logger.h"
#include "io_buf.h"
#include "cb_continuous_label.h"
#include <cfloat>

namespace CB
{
template <typename LabelT = CB::label>
size_t read_cached_label_additional_fields(LabelT& ld, io_buf& cache)
{
  ld.weight = cache.read_value<float>("weight");
  return sizeof(float);
}

template <typename LabelT = CB::label, typename LabelElmT = cb_class>
size_t read_cached_label(LabelT& ld, io_buf& cache)
{
  ld.costs.clear();

  // It would be desirable to be able to use cache.read_value here as below:
  //
  // size_t num_costs = cache.read_value<size_t>("ld.costs.size()");
  //
  // Unfortunately, it seems like for newline examples (e.g. terminal
  // example) what happens is that the cache ends, and the label parser
  // is expected to silently do nothing.
  // Previously we would just return 0 here if we could not read anything.
  char* c;
  if (cache.buf_read(c, sizeof(size_t)) < sizeof(size_t)) return 0;
  size_t num_costs = *reinterpret_cast<size_t*>(c);
  c += sizeof(size_t);
  cache.set(c);

  for (size_t i = 0; i < num_costs; i++) { ld.costs.push_back(cache.read_value<LabelElmT>("ld.costs[i]")); }

  size_t total =
      sizeof(size_t) + num_costs * sizeof(LabelElmT) + read_cached_label_additional_fields<LabelT>(ld, cache);

  return total;
}

template <typename LabelT = CB::label>
void cache_label_additional_fields(const LabelT& ld, io_buf& cache)
{
  cache.write_value(ld.weight);
}

template <typename LabelT = CB::label, typename LabelElmT = cb_class>
void cache_label(const LabelT& ld, io_buf& cache)
{
  cache.write_value<size_t>(ld.costs.size());

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
