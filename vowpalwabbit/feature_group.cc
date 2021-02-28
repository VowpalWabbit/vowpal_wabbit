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

// features::features()
// {
//   values = v_init<feature_value>();
//   indicies = v_init<feature_index>();
//   sum_feat_sq = 0.f;
// }

// features::~features()
// {
//   values.delete_v();
//   indicies.delete_v();
// }

// features::features(features&& other) noexcept
//     : values(std::move(other.values))
//     , indicies(std::move(other.indicies))
//     , space_names(std::move(other.space_names))
//     , sum_feat_sq(other.sum_feat_sq)
// {
//   other.sum_feat_sq = 0;
// }

// features& features::operator=(features&& other) noexcept
// {
//   values = std::move(other.values);
//   indicies = std::move(other.indicies);
//   space_names = std::move(other.space_names);
//   sum_feat_sq = other.sum_feat_sq;
//   other.sum_feat_sq = 0;
//   return *this;
// }

void features::free_space_names(size_t i) { space_names.erase(space_names.begin() + i, space_names.end()); }

void features::clear()
{
  sum_feat_sq = 0.f;
  values.clear();
  indicies.clear();
  space_names.clear();
}

void features::truncate_to(const features_value_iterator& pos)
{
  auto i = pos._begin - values.begin();
  values.end() = pos._begin;
  if (indicies.end() != indicies.begin()) { indicies.end() = indicies.begin() + i; }

  if (space_names.begin() != space_names.end()) { space_names.erase(space_names.begin() + i, space_names.end()); }
}

void features::truncate_to(size_t i)
{
  values.end() = values.begin() + i;
  if (indicies.end() != indicies.begin()) { indicies.end() = indicies.begin() + i; }

  if (space_names.size() > i) { space_names.erase(space_names.begin() + i, space_names.end()); }
}

void features::push_back(feature_value v, feature_index i)
{
  values.push_back(v);
  indicies.push_back(i);
  sum_feat_sq += v * v;
}

bool features::sort(uint64_t parse_mask)
{
  if (indicies.empty()) { return false; }

  if (!space_names.empty())
  {
    std::vector<feature_slice> slice;
    slice.reserve(indicies.size());
    for (size_t i = 0; i < indicies.size(); i++)
    { slice.push_back({values[i], indicies[i] & parse_mask, *space_names[i]}); }
    // The comparator should return true if the first element is less than the second.
    std::sort(slice.begin(), slice.end(), [](const feature_slice& first, const feature_slice& second) {
      return (first.weight_index < second.weight_index) ||
          ((first.weight_index == second.weight_index) && (first.x < second.x));
    });

    for (size_t i = 0; i < slice.size(); i++)
    {
      values[i] = slice[i].x;
      indicies[i] = slice[i].weight_index;
      *space_names[i] = slice[i].space_name;
    }
  }
  else
  {
    std::vector<feature> slice;
    slice.reserve(indicies.size());

    for (size_t i = 0; i < indicies.size(); i++) { slice.emplace_back(values[i], indicies[i] & parse_mask); }
    // The comparator should return true if the first element is less than the second.
    std::sort(slice.begin(), slice.end(), [](const feature& first, const feature& second) {
      return (first.weight_index < second.weight_index) ||
          ((first.weight_index == second.weight_index) && (first.x < second.x));
    });
    for (size_t i = 0; i < slice.size(); i++)
    {
      values[i] = slice[i].x;
      indicies[i] = slice[i].weight_index;
    }
  }
  return true;
}

void features::deep_copy_from(const features& src)
{
  copy_array(values, src.values);
  copy_array(indicies, src.indicies);
  space_names = src.space_names;
  sum_feat_sq = src.sum_feat_sq;
}
