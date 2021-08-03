// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "feature_group.h"

#include "v_array.h"

#include <vector>
#include <algorithm>
#include <utility>

struct feature_slice  // a helper struct for functions using the set {v,i,space_name}
{
  feature_value x;
  feature_index weight_index;
  audit_strings space_name;
};

void features::free_space_names(size_t i) { space_names.erase(space_names.begin() + i, space_names.end()); }

void features::clear()
{
  sum_feat_sq = 0.f;
  values.clear();
  indicies.clear();
  space_names.clear();
}

void features::truncate_to(const iterator& pos) { truncate_to(std::distance(begin(), pos)); }

void features::truncate_to(const audit_iterator& pos) { truncate_to(std::distance(audit_begin(), pos)); }

void features::truncate_to(size_t i)
{
  values.resize_but_with_stl_behavior(i);
  if (indicies.end() != indicies.begin()) { indicies.resize_but_with_stl_behavior(i); }

  if (space_names.size() > i) { space_names.erase(space_names.begin() + i, space_names.end()); }
}

void features::concat(const features& other)
{
  if (other.empty()) { return; }

  // Conditions to check:
  //  - !empty() && audit && other.audit -> push val, idx, audit
  //  - !empty() && audit && !other.audit -> fail
  //  - !empty() && !audit && other.audit -> fail
  //  - !empty() && !audit && !other.audit -> push val, idx
  //  - empty() && other.audit -> push val, idx, audit
  //  - empty() && !other.audit -> push val, idx

  if (!empty() && (space_names.empty() != other.space_names.empty()))
  { THROW_OR_RETURN_VOID("Cannot merge two feature groups if one has audit info and the other does not."); }
  values.insert(values.end(), other.values.begin(), other.values.end());
  indicies.insert(indicies.end(), other.indicies.begin(), other.indicies.end());

  if (!other.space_names.empty())
  { space_names.insert(space_names.end(), other.space_names.begin(), other.space_names.end()); }
}

void features::push_back(feature_value v, feature_index i)
{
  values.push_back(v);
  indicies.push_back(i);
  sum_feat_sq += v * v;
}

// https://stackoverflow.com/questions/17074324/how-can-i-sort-two-vectors-in-the-same-way-with-criteria-that-uses-only-one-of
template <typename IndexVec, typename ValVec, typename Compare>
std::vector<std::size_t> sort_permutation(const IndexVec& index_vec, const ValVec& value_vec, Compare& compare)
{
  std::vector<std::size_t> p(vec.size());
  std::iota(p.begin(), p.end(), 0);
  std::sort(p.begin(), p.end(),
      [&](std::size_t i, std::size_t j) { return compare(index_vec[i], index_vec[j], value_vec[i], value_vec[j]); });
  return p;
}

template <typename VecT>
void apply_permutation_in_place(VecT& vec, const std::vector<std::size_t>& p)
{
  std::vector<bool> done(vec.size());
  for (std::size_t i = 0; i < vec.size(); ++i)
  {
    if (done[i]) { continue; }
    done[i] = true;
    std::size_t prev_j = i;
    std::size_t j = p[i];
    while (i != j)
    {
      std::swap(vec[prev_j], vec[j]);
      done[j] = true;
      prev_j = j;
      j = p[j];
    }
  }
}

bool features::sort()
{
  if (indicies.empty()) { return false; }

  const auto comparator = [](feature_value value_first, feature_value value_second, feature_index index_first,
                              feature_index index_second) {
    return (index_first < index_second) || ((index_first == index_second) && (value_first < value_second));
  };
  auto p = sort_permutation(values, indicies, comparator);
  apply_permutation_in_place(values, p);
  apply_permutation_in_place(indicies, p);
  apply_permutation_in_place(space_names, p);
  return true;
}



void features::deep_copy_from(const features& src)
{
  values = src.values;
  indicies = src.indicies;
  space_names = src.space_names;
  sum_feat_sq = src.sum_feat_sq;
}
