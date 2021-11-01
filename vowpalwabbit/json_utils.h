#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "feature_group.h"
#include "global_data.h"
#include "hash.h"
#include "vw.h"

// Decision Service JSON header information - required to construct final label
struct DecisionServiceInteraction
{
  std::string eventId;
  std::string timestamp;
  std::vector<unsigned> actions;
  std::vector<float> probabilities;
  std::vector<unsigned> baseline_actions;
  float probabilityOfDrop = 0.f;
  float originalLabelCost = 0.f;
  float originalLabelCostFirstSlot = 0.f;
  bool skipLearn{false};
};

template <bool audit>
struct Namespace
{
  char feature_group;
  feature_index namespace_hash;
  features* ftrs;
  size_t feature_count;
  const char* name;

  void AddFeature(feature_value v, feature_index i, const char* feature_name)
  {
    // filter out 0-values
    if (v == 0) return;

    ftrs->push_back(v, i);
    feature_count++;

    if (audit) ftrs->space_names.push_back(audit_strings(name, feature_name));
  }

  void AddFeature(const char* str, hash_func_t hash_func, uint64_t parse_mask)
  {
    auto hashed_feature = hash_func(str, strlen(str), namespace_hash) & parse_mask;
    ftrs->push_back(1., hashed_feature);
    feature_count++;

    if (audit) ftrs->space_names.push_back(audit_strings(name, str));
  }

  void AddFeature(const char* key, const char* value, hash_func_t hash_func, uint64_t parse_mask)
  {
    ftrs->push_back(1., VW::chain_hash_static(key, value, namespace_hash, hash_func, parse_mask));
    feature_count++;

    std::stringstream ss;
    ss << key << "^" << value;
    if (audit) ftrs->space_names.push_back(audit_strings(name, ss.str()));
  }
};

template <bool audit>
void push_ns(example* ex, const char* ns, std::vector<Namespace<audit>>& namespaces, hash_func_t hash_func, uint64_t hash_seed)
{
  Namespace<audit> n;
  n.feature_group = ns[0];
  n.namespace_hash = hash_func(ns, strlen(ns), hash_seed);
  n.ftrs = ex->feature_space.data() + ns[0];
  n.feature_count = 0;
  n.name = ns;

  if (!namespaces.empty())
  {
    // Close last
    auto& top = namespaces.back();
    if (!top.ftrs->namespace_extents.empty() && top.ftrs->namespace_extents.back().end_index == 0)
    { top.ftrs->end_ns_extent(); }
  }
  // Add new
  n.ftrs->start_ns_extent(n.namespace_hash);

  namespaces.push_back(std::move(n));
}

template <bool audit>
void pop_ns(example* ex, std::vector<Namespace<audit>>& namespaces)
{
  auto& ns = namespaces.back();
  if (ns.feature_count > 0)
  {
    auto feature_group = ns.feature_group;
    // Do not insert feature_group if it already exists.
    if (std::find(ex->indices.begin(), ex->indices.end(), feature_group) == ex->indices.end())
    { ex->indices.push_back(feature_group); }
  }

  ns.ftrs->end_ns_extent();
  namespaces.pop_back();

  if (!namespaces.empty())
  {
    auto& top = namespaces.back();
    top.ftrs->start_ns_extent(top.namespace_hash);
  }
}
