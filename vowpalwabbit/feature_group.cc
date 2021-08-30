// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "feature_group.h"

#include "v_array.h"

#include <vector>
#include <algorithm>
#include <utility>
#include <numeric>

struct feature_slice  // a helper struct for functions using the set {v,i,space_name}
{
  feature_value x;
  feature_index weight_index;
  audit_strings space_name;
};

void features::clear()
{
  sum_feat_sq = 0.f;
  values.clear();
  indicies.clear();
  space_names.clear();
}

void features::truncate_to(const audit_iterator& pos, float sum_feat_sq_of_removed_section)
{
  truncate_to(std::distance(audit_begin(), pos), sum_feat_sq_of_removed_section);
}

void features::truncate_to(const iterator& pos, float sum_feat_sq_of_removed_section)
{
  truncate_to(std::distance(begin(), pos), sum_feat_sq_of_removed_section);
}

void features::truncate_to(size_t i, float sum_feat_sq_of_removed_section)
{
  assert(i <= size());
  if (i == size()) { return; }
  sum_feat_sq -= sum_feat_sq_of_removed_section;

  values.resize_but_with_stl_behavior(i);
  if (indicies.end() != indicies.begin()) { indicies.resize_but_with_stl_behavior(i); }

  if (space_names.size() > i) { space_names.erase(space_names.begin() + i, space_names.end()); }
}

void features::truncate_to(const audit_iterator& pos) { truncate_to(std::distance(audit_begin(), pos)); }
void features::truncate_to(const iterator& pos) { truncate_to(std::distance(begin(), pos)); }
void features::truncate_to(size_t i)
{
  assert(i <= size());
  float sum_ft_squares_of_removed_chunk = 0.f;
  for (auto idx = i; idx < values.size(); ++idx) { sum_ft_squares_of_removed_chunk += values[idx] * values[i]; }
  truncate_to(i, sum_ft_squares_of_removed_chunk);
}

void features::concat(const features& other)
{
  assert(values.size() == indicies.size());
  assert(other.values.size() == other.indicies.size());
  // Conditions to check:
  //  - !empty() && audit && other.audit -> push val, idx, audit
  //  - !empty() && audit && !other.audit -> fail
  //  - !empty() && !audit && other.audit -> fail
  //  - !empty() && !audit && !other.audit -> push val, idx
  //  - empty() && other.audit -> push val, idx, audit
  //  - empty() && !other.audit -> push val, idx

  // Cannot merge two feature groups if one has audit info and the other does not.
  assert(!(!empty() && (space_names.empty() != other.space_names.empty())));
  sum_feat_sq += other.sum_feat_sq;

  for (size_t i = 0; i < other.size(); ++i)
  {
    values.push_back(other.values[i]);
    indicies.push_back(other.indicies[i]);
  }

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
std::vector<std::size_t> sort_permutation(const IndexVec& index_vec, const ValVec& value_vec, const Compare& compare)
{
  assert(index_vec.size() == value_vec.size());
  std::vector<std::size_t> dest_index_vec(index_vec.size());
  std::iota(dest_index_vec.begin(), dest_index_vec.end(), 0);
  std::sort(dest_index_vec.begin(), dest_index_vec.end(),
      [&](std::size_t i, std::size_t j) { return compare(index_vec[i], index_vec[j], value_vec[i], value_vec[j]); });
  return dest_index_vec;
}

template <typename VecT>
void apply_permutation_in_place(VecT& vec, const std::vector<std::size_t>& dest_index_vec)
{
  std::vector<bool> done(vec.size());
  for (std::size_t i = 0; i < vec.size(); ++i)
  {
    if (done[i]) { continue; }
    done[i] = true;
    std::size_t prev_j = i;
    std::size_t j = dest_index_vec[i];
    while (i != j)
    {
      std::swap(vec[prev_j], vec[j]);
      done[j] = true;
      prev_j = j;
      j = dest_index_vec[j];
    }
  }
}

bool features::sort(uint64_t parse_mask)
{
  if (indicies.empty()) { return false; }
  // Compared indices are masked even though the saved values are not necessarilly masked.
  const auto comparator = [parse_mask](feature_value value_first, feature_value value_second, feature_index index_first,
                              feature_index index_second) {
    auto masked_index_first = index_first & parse_mask;
    auto masked_index_second = index_second & parse_mask;
    return (masked_index_first < masked_index_second) ||
        ((masked_index_first == masked_index_second) && (value_first < value_second));
  };
  auto dest_index_vec = sort_permutation(values, indicies, comparator);
  apply_permutation_in_place(values, dest_index_vec);
  apply_permutation_in_place(indicies, dest_index_vec);
  if (!space_names.empty()) { apply_permutation_in_place(space_names, dest_index_vec); }
  return true;
}
