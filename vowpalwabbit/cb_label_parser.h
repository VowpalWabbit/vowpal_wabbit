// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once
#include "cb.h"

namespace CB
{
template <typename LabelT = CB::label>
char* bufread_label_additional_fields(LabelT& ld, char* c)
{
  memcpy(&ld.weight, c, sizeof(ld.weight));
  c += sizeof(ld.weight);
  return c;
}

template <typename LabelT = CB::label, typename LabelElmT = cb_class>
char* bufread_label(LabelT& ld, char* c, io_buf& cache)
{
  size_t num = *(size_t*)c;
  ld.costs.clear();
  c += sizeof(size_t);
  size_t total = sizeof(LabelElmT) * num;
  if (cache.buf_read(c, total) < total)
  {
    std::cout << "error in demarshal of cost data" << std::endl;
    return c;
  }
  for (size_t i = 0; i < num; i++)
  {
    LabelElmT temp = *(LabelElmT*)c;
    c += sizeof(LabelElmT);
    ld.costs.push_back(temp);
  }

  return bufread_label_additional_fields(ld, c);
}

template <typename LabelT = CB::label, typename LabelElmT = cb_class>
size_t read_cached_label(shared_data*, LabelT& ld, io_buf& cache)
{
  ld.costs.clear();
  char* c;
  size_t total = sizeof(size_t);
  if (cache.buf_read(c, total) < total) return 0;
  bufread_label<LabelT, LabelElmT>(ld, c, cache);

  return total;
}

template <typename LabelT>
char* bufcache_label_additional_fields(LabelT& ld, char* c)
{
  memcpy(c, &ld.weight, sizeof(ld.weight));
  c += sizeof(ld.weight);
  return c;
}

template <typename LabelT = CB::label, typename LabelElmT = cb_class>
char* bufcache_label(LabelT& ld, char* c)
{
  *(size_t*)c = ld.costs.size();
  c += sizeof(size_t);
  for (size_t i = 0; i < ld.costs.size(); i++)
  {
    *(LabelElmT*)c = ld.costs[i];
    c += sizeof(LabelElmT);
  }
  return bufcache_label_additional_fields(ld, c);
}

template <typename LabelT = CB::label, typename LabelElmT = cb_class>
void cache_label(LabelT& ld, io_buf& cache)
{
  char* c;
  cache.buf_write(c, sizeof(size_t) + sizeof(LabelElmT) * ld->costs.size());
  bufcache_label<LabelT, LabelElmT>(ld, c);
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
float get_probability(LabelElmT& elm)
{
  return elm.probability;
}

template <>
inline float get_probability(VW::cb_continuous::continuous_label_elm& elm)
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

template <typename LabelT = CB::label>
void delete_label(LabelT& ld)
{
  ld.costs.delete_v();
}

template <typename LabelT = CB::label>
void copy_label_additional_fields(LabelT& dst, LabelT& src)
{
  dst.weight = src.weight;
}

template <typename LabelT = CB::label>
void copy_label(LabelT& dst, LabelT& src)
{
  copy_array(ldD.costs, ldS.costs);
  copy_label_additional_fields(ldD, ldS);
}
}  // namespace CB
