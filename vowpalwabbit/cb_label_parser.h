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
template <typename LabelT = CB::label, typename LabelElmT = cb_class>
char* bufread_label_elements(LabelT& ld, char* c, io_buf& cache)
{
  size_t num = *reinterpret_cast<size_t*>(c);
  ld.costs.clear();
  c += sizeof(size_t);
  size_t total = sizeof(LabelElmT) * num;
  if (cache.buf_read(c, total) < total) { THROW("error in demarshal of cost data"); }
  for (size_t i = 0; i < num; i++)
  {
    LabelElmT temp = *(LabelElmT*)c;
    c += sizeof(LabelElmT);
    ld.costs.push_back(temp);
  }

  return c;
}

template <typename LabelT = CB::label>
size_t read_cached_label_additional_fields(LabelT& ld, io_buf& cache)
{
  ld.weight = cache.read_value<float>("weight");
  return sizeof(float);
}

template <typename LabelT = CB::label, typename LabelElmT = cb_class>
size_t read_cached_label(shared_data*, LabelT& ld, io_buf& cache)
{
  ld.costs.clear();
  char* c;
  size_t total = sizeof(size_t);
  if (cache.buf_read(c, total) < total) return 0;
  c = bufread_label_elements<LabelT, LabelElmT>(ld, c, cache);
  cache.set(c);

  total += read_cached_label_additional_fields<LabelT>(ld, cache);

  return total;
}

template <typename LabelT = CB::label, typename LabelElmT = cb_class>
char* bufcache_label_elements(LabelT& ld, char* c)
{
  *reinterpret_cast<size_t*>(c) = ld.costs.size();
  c += sizeof(size_t);
  for (size_t i = 0; i < ld.costs.size(); i++)
  {
    *(LabelElmT*)c = ld.costs[i];
    c += sizeof(LabelElmT);
  }

  return c;
}

template <typename LabelT = CB::label>
void cache_label_additional_fields(LabelT& ld, io_buf& cache)
{
  cache.write_value(ld.weight);
}

template <typename LabelT = CB::label, typename LabelElmT = cb_class>
void cache_label(LabelT& ld, io_buf& cache)
{
  char* c;
  cache.buf_write(c, sizeof(size_t) + sizeof(LabelElmT) * ld.costs.size());
  c = bufcache_label_elements<LabelT, LabelElmT>(ld, c);
  cache.set(c);

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
bool is_test_label(LabelT& ld)
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
