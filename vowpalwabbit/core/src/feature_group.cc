// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/feature_group.h"

#include "vw/core/v_array.h"

#include <algorithm>
#include <numeric>
#include <utility>
#include <vector>

void VW::features::clear()
{
  sum_feat_sq = 0.f;
  values.clear();
  indices.clear();
  space_names.clear();
  namespace_extents.clear();
}

void VW::features::truncate_to(const audit_iterator& pos, float sum_feat_sq_of_removed_section)
{
  truncate_to(std::distance(audit_begin(), pos), sum_feat_sq_of_removed_section);
}

void VW::features::truncate_to(const iterator& pos, float sum_feat_sq_of_removed_section)
{
  truncate_to(std::distance(begin(), pos), sum_feat_sq_of_removed_section);
}

void VW::features::truncate_to(size_t i, float sum_feat_sq_of_removed_section)
{
  assert(i <= size());
  if (i == size()) { return; }
  sum_feat_sq -= sum_feat_sq_of_removed_section;

  values.resize(i);
  if (indices.end() != indices.begin()) { indices.resize(i); }

  if (space_names.size() > i) { space_names.erase(space_names.begin() + i, space_names.end()); }

  while (!namespace_extents.empty() && namespace_extents.back().begin_index >= i) { namespace_extents.pop_back(); }

  // Check if the truncation cuts an extent in the middle.
  if (!namespace_extents.empty())
  {
    if (namespace_extents.back().end_index > i) { namespace_extents.back().end_index = i; }
  }
}

void VW::features::truncate_to(const audit_iterator& pos) { truncate_to(std::distance(audit_begin(), pos)); }
void VW::features::truncate_to(const iterator& pos) { truncate_to(std::distance(begin(), pos)); }
void VW::features::truncate_to(size_t i)
{
  assert(i <= size());
  float sum_ft_squares_of_removed_chunk = 0.f;
  for (auto idx = i; idx < values.size(); ++idx) { sum_ft_squares_of_removed_chunk += values[idx] * values[i]; }
  truncate_to(i, sum_ft_squares_of_removed_chunk);
}

void VW::features::concat(const features& other)
{
  assert(values.size() == indices.size());
  assert(other.values.size() == other.indices.size());
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

  const auto extent_offset = indices.size();
  for (size_t i = 0; i < other.size(); ++i)
  {
    values.push_back(other.values[i]);
    indices.push_back(other.indices[i]);
  }

  if (!other.space_names.empty())
  {
    space_names.insert(space_names.end(), other.space_names.begin(), other.space_names.end());
  }

  // If the back of the current list and the front of the other list have the same hash then merge the extent.
  size_t offset = 0;
  if (!namespace_extents.empty() && !other.namespace_extents.empty() &&
      (namespace_extents.back().hash == other.namespace_extents.front().hash))
  {
    namespace_extents.back().end_index +=
        (other.namespace_extents.front().end_index - other.namespace_extents.front().begin_index);
    offset = 1;
  }

  for (size_t i = offset; i < other.namespace_extents.size() - offset; ++i)
  {
    const auto& ns_extent = other.namespace_extents[i];
    namespace_extents.emplace_back(
        ns_extent.begin_index + extent_offset, ns_extent.end_index + extent_offset, ns_extent.hash);
  }
}

void VW::features::push_back(feature_value v, feature_index i)
{
  values.push_back(v);
  indices.push_back(i);
  sum_feat_sq += v * v;
}

void VW::features::push_back(feature_value v, feature_index i, uint64_t hash)
{
  // If there is an open extent but of a different hash - we must close it before we do anything.
  if (!namespace_extents.empty() && namespace_extents.back().hash != hash && (namespace_extents.back().end_index == 0))
  {
    end_ns_extent();
  }

  // We only need to extend the extent if it has had its end index set. If the end index is 0, then we assume the extent
  // is open and will be closed before the example is finished being constructed.
  const bool should_extend_existing =
      !namespace_extents.empty() && namespace_extents.back().hash == hash && namespace_extents.back().end_index != 0;
  // If there is an extent but of a different hash - we must add a new one.
  const bool should_create_new = namespace_extents.empty() || namespace_extents.back().hash != hash;

  if (should_extend_existing) { namespace_extents.back().end_index++; }
  else if (should_create_new) { namespace_extents.emplace_back(indices.size(), indices.size() + 1, hash); }

  values.push_back(v);
  indices.push_back(i);
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

template <typename VecT, typename... Rest>
size_t size_of_first_vec(const VecT& vec, Rest&... /*rest*/)
{
  return vec.size();
}

template <typename VecT>
void do_swap_for_all(size_t pos1, size_t pos2, VecT& vec)
{
  std::swap(vec[pos1], vec[pos2]);
}

template <typename VecT, typename... Rest>
void do_swap_for_all(size_t pos1, size_t pos2, VecT& vec, Rest&... rest)
{
  std::swap(vec[pos1], vec[pos2]);
  do_swap_for_all(pos1, pos2, rest...);
}

template <typename... VecTs>
void apply_permutation_in_place(const std::vector<std::size_t>& dest_index_vec, VecTs&... vecs)
{
  const auto size = size_of_first_vec(vecs...);
  assert(dest_index_vec.size() == size);
  std::vector<bool> done(size);
  for (std::size_t i = 0; i < size; ++i)
  {
    if (done[i]) { continue; }
    done[i] = true;
    std::size_t prev_j = i;
    std::size_t j = dest_index_vec[i];
    while (i != j)
    {
      do_swap_for_all(prev_j, j, vecs...);
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
// This function converts the list of ranges into a flat list of elements
// corresponding to each index in the given list of ranges. The
// overall_feature_space_size is used to fill th elements between th last range
// ending and the end of the overall logical list. This function is used to
// perform elementwise operations such as filtering and sorting when combined
// with the other vectors in a features object.
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

// This function can take the output of flatten_namespace_extents and reverse it
// to get back to the original representation required in the features object.
std::vector<namespace_extent> unflatten_namespace_extents(const std::vector<std::pair<bool, uint64_t>>& extents)
{
  if (extents.empty()) { return {}; }
  std::vector<namespace_extent> results;
  auto last_start = std::size_t{0};
  auto current = extents[0];
  for (auto i = std::size_t{1}; i < extents.size(); ++i)
  {
    if (current != extents[i])
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

bool VW::features::sort(uint64_t parse_mask)
{
  if (indices.empty()) { return false; }
  // Compared indices are masked even though the saved values are not necessarilly masked.
  const auto comparator = [parse_mask](feature_index index_first, feature_index index_second, feature_value value_first,
                              feature_value value_second)
  {
    const auto masked_index_first = index_first & parse_mask;
    const auto masked_index_second = index_second & parse_mask;
    return (masked_index_first < masked_index_second) ||
        ((masked_index_first == masked_index_second) && (value_first < value_second));
  };
  auto flat_extents = VW::details::flatten_namespace_extents(namespace_extents, indices.size());
  const auto dest_index_vec = sort_permutation(indices, values, comparator);
  if (!space_names.empty()) { apply_permutation_in_place(dest_index_vec, values, indices, flat_extents, space_names); }
  else { apply_permutation_in_place(dest_index_vec, values, indices, flat_extents); }
  namespace_extents = VW::details::unflatten_namespace_extents(flat_extents);
  return true;
}

void VW::features::start_ns_extent(uint64_t hash)
{
  // Either the list should be empty or the last one should have a valid end index.
  assert(namespace_extents.empty() || (!namespace_extents.empty() && namespace_extents.back().end_index != 0));
  namespace_extents.emplace_back(indices.size(), hash);
}

void VW::features::end_ns_extent()
{
  // There should have been an extent started.
  assert(!namespace_extents.empty());
  // If the last extent has already been ended this is an error.
  assert(namespace_extents.back().end_index == 0);

  const auto end_index = indices.size();
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
