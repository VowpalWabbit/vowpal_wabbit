#pragma once

#include <string>
#include <vector>

#include "feature_group.h"
#include "namespaced_features.h"
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
  float probabilityOfDrop = 0.f;
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

    if (audit) ftrs->space_names.push_back(audit_strings_ptr(new audit_strings(name, feature_name)));
  }

  void AddFeature(vw* all, const char* str)
  {
    ftrs->push_back(1., VW::hash_feature_cstr(*all, const_cast<char*>(str), namespace_hash));
    feature_count++;

    if (audit) ftrs->space_names.push_back(audit_strings_ptr(new audit_strings(name, str)));
  }

  void AddFeature(vw* all, const char* key, const char* value)
  {
    ftrs->push_back(1., VW::chain_hash(*all, key, value, namespace_hash));
    feature_count++;

    std::stringstream ss;
    ss << key << "^" << value;
    if (audit) ftrs->space_names.push_back(audit_strings_ptr(new audit_strings(name, ss.str())));
  }
};

inline void remove_empty_namespaces(VW::namespaced_features& feature_space)
{
  std::vector<uint64_t> hashes_to_remove;
  for (auto it = feature_space.begin(); it != feature_space.end(); ++it)
  {
    if ((*it).empty()) { hashes_to_remove.push_back(it.hash()); }
  }
  for (auto hash : hashes_to_remove) { feature_space.remove_feature_group(hash); }
}

template <bool audit>
void push_ns(example* ex, const char* ns, std::vector<Namespace<audit>>& namespaces, vw& all)
{
  Namespace<audit> n;
  n.feature_group = ns[0];
  n.namespace_hash = VW::hash_space_cstr(all, ns);
  n.ftrs = &ex->feature_space.get_or_create_feature_group(n.namespace_hash, ns[0]);
  n.feature_count = 0;
  n.name = ns;
  namespaces.push_back(std::move(n));
}


template <bool audit>
void pop_ns(example* ex, std::vector<Namespace<audit>>& namespaces)
{
  namespaces.pop_back();
}
