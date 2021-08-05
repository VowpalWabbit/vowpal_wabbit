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

void features::free_space_names(size_t i) { space_names.erase(space_names.begin() + i, space_names.end()); }

void features::clear()
{
  sum_feat_sq = 0.f;
  values.clear();
  indicies.clear();
  space_names.clear();
  namespace_extents.clear();
}

void features::truncate_to(const iterator& pos) { truncate_to(std::distance(begin(), pos)); }

void features::truncate_to(const audit_iterator& pos) { truncate_to(std::distance(audit_begin(), pos)); }

void features::truncate_to(size_t i)
{
  values.resize_but_with_stl_behavior(i);
  if (indicies.end() != indicies.begin()) { indicies.resize_but_with_stl_behavior(i); }

  if (space_names.size() > i) { space_names.erase(space_names.begin() + i, space_names.end()); }

  while (!namespace_extents.empty() && namespace_extents.back().begin_index >= i) { namespace_extents.pop_back(); }

  // Check if the truncation cuts an extent in the middle.
  if (!namespace_extents.empty())
  {
    if (namespace_extents.back().end_index > i) { namespace_extents.back().end_index = i; }
  }
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

  const auto extent_offset = indicies.size();
  if (!empty() && (space_names.empty() != other.space_names.empty()))
  {
    THROW_OR_RETURN_VOID("Cannot merge two feature groups if one has audit info and the other does not.");
  }
  values.insert(values.end(), other.values.begin(), other.values.end());
  indicies.insert(indicies.end(), other.indicies.begin(), other.indicies.end());

  if (!other.space_names.empty())
  {
    space_names.insert(space_names.end(), other.space_names.begin(), other.space_names.end());
  }

  for (const auto& ns_extent : other.namespace_extents)
  {
    namespace_extents.emplace_back(
        ns_extent.begin_index + extent_offset, ns_extent.end_index + extent_offset, ns_extent.hash);
  }
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

void push_many(std::vector<std::pair<bool, uint64_t>>& vec, size_t num, bool is_valid, uint64_t hash)
{
  for (auto i = std::size_t{0}; i < num; ++i) { vec.emplace_back(is_valid, hash); }
}

namespace VW
{
namespace details
{
std::vector<std::pair<bool, uint64_t>> flatten_namespace_extents(
    const std::vector<namespace_extent>& extents, size_t overall_feature_space_size)
{
  assert(extents.empty() || (overall_feature_space_size >= extents.back().end_index));
  std::vector<std::pair<bool, uint64_t>> flattened;
  flattened.reserve(overall_feature_space_size);
  auto last_end = std::size_t{0};
  for (const auto& extent : extents)
  {
    if (extent.begin_index > last_end) { push_many(flattened, extent.begin_index - last_end, false, 0); }
    push_many(flattened, extent.end_index - extent.begin_index, true, extent.hash);
    last_end = extent.end_index;
  }
  if (overall_feature_space_size > last_end) { push_many(flattened, overall_feature_space_size - last_end, false, 0); }
  return flattened;
}

std::vector<namespace_extent> unflatten_namespace_extents(const std::vector<std::pair<bool, uint64_t>>& extents)
{
  if (extents.empty()) { return {}; }
  std::vector<namespace_extent> results;
  auto eq = [](const std::pair<bool, uint64_t>& left, const std::pair<bool, uint64_t>& right) {
    return left.first == right.first && left.second == right.second;
  };
  auto last_start = std::size_t{0};
  auto current = extents[0];
  for (auto i = std::size_t{1}; i < extents.size(); ++i)
  {
    if (!eq(current, extents[i]))
    {
      // Check if it was a valid sequence, or an empty segment.
      if (current.first) { results.emplace_back(last_start, i, current.second); }
      last_start = i;
      current = extents[i];
    }
  }

  if (current.first) { results.emplace_back(last_start, extents.size(), current.second); }
  return results;
}
}  // namespace details
}  // namespace VW

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

  auto flat_extents = VW::details::flatten_namespace_extents(namespace_extents, indicies.size());
  apply_permutation_in_place(flat_extents, dest_index_vec);
  namespace_extents = VW::details::unflatten_namespace_extents(flat_extents);
  return true;
}

void features::start_ns_extent(uint64_t hash)
{
  // Either the list should be empty or the last one should have a valid end index.
  assert(namespace_extents.empty() || (!namespace_extents.empty() && namespace_extents.back().end_index != 0));
  namespace_extents.emplace_back(indicies.size(), hash);
}

void features::end_ns_extent()
{
  // There should have been an extent started.
  assert(!namespace_extents.empty());
  // If the last extent has already been ended this is an error.
  assert(namespace_extents.back().end_index == 0);

  const auto end_index = indicies.size();
  namespace_extents.back().end_index = end_index;

  // If the size of the extent is empty then just remove it.
  if (namespace_extents.back().begin_index == namespace_extents.back().end_index) { namespace_extents.pop_back(); }

  // If the most recent extent is actually the same as the previous one, merge them.
  if (namespace_extents.size() > 1)
  {
    auto& prev = namespace_extents[namespace_extents.size() - 2];
    if (prev.hash == namespace_extents.back().hash)
    {
      prev.end_index = end_index;
      namespace_extents.pop_back();
    }
  }
}

void features::deep_copy_from(const features& src)
{
  values = src.values;
  indicies = src.indicies;
  space_names = src.space_names;
  sum_feat_sq = src.sum_feat_sq;
  namespace_extents = src.namespace_extents;
}
