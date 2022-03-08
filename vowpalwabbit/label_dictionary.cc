// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "label_dictionary.h"

#include "example.h"

namespace LabelDict
{
void del_example_namespace(example& ec, namespace_index ns, features& fs)
{
  // print_update is called after this del_example_namespace,
  // so we need to keep the ec.num_features correct,
  // so shared features are included in the reported number of "current features"
  // ec.num_features -= numf;
  features& del_target = ec.feature_space[static_cast<size_t>(ns)];
  assert(del_target.size() >= fs.size());
  assert(!ec.indices.empty());
  if (ec.indices.back() == ns && ec.feature_space[static_cast<size_t>(ns)].size() == fs.size())
  { ec.indices.pop_back(); }
  ec.reset_total_sum_feat_sq();
  ec.num_features -= fs.size();
  del_target.truncate_to(del_target.size() - fs.size(), fs.sum_feat_sq);
}

void add_example_namespace(example& ec, namespace_index ns, features& fs)
{
  const auto index_it = std::find(ec.indices.begin(), ec.indices.end(), ns);
  const bool has_ns = index_it != ec.indices.end();
  if (!has_ns) ec.indices.push_back(ns);

  features& add_fs = ec.feature_space[static_cast<size_t>(ns)];
  add_fs.concat(fs);
  ec.reset_total_sum_feat_sq();
  ec.num_features += fs.size();
}

void add_example_namespaces_from_example(example& target, example& source)
{
  for (namespace_index idx : source.indices)
  {
    if (idx == constant_namespace) continue;
    add_example_namespace(target, idx, source.feature_space[idx]);
  }
}

void del_example_namespaces_from_example(example& target, example& source)
{
  if (source.indices.empty())  // making sure we can deal with empty shared example
    return;
  namespace_index* idx = source.indices.end();
  idx--;
  for (; idx >= source.indices.begin(); idx--)
  {
    if (*idx == constant_namespace) continue;
    del_example_namespace(target, *idx, source.feature_space[*idx]);
  }
}

void add_example_namespace_from_memory(label_feature_map& lfm, example& ec, size_t lab)
{
  auto res_iter = lfm.find(lab);
  if (res_iter == lfm.end()) return;
  add_example_namespace(ec, static_cast<namespace_index>('l'), res_iter->second);
}

void del_example_namespace_from_memory(label_feature_map& lfm, example& ec, size_t lab)
{
  auto res_iter = lfm.find(lab);
  if (res_iter == lfm.end()) return;
  del_example_namespace(ec, static_cast<namespace_index>('l'), res_iter->second);
}

void set_label_features(label_feature_map& lfm, size_t lab, features& fs)
{
  if (lfm.find(lab) == lfm.end()) return;
  lfm.emplace(lab, fs);
}

}  // namespace LabelDict