#pragma once

#include "example_predict.h"
#include "v_hashmap.h"

namespace LabelDict
{
typedef v_hashmap<size_t, features> label_feature_map;
inline bool size_t_eq(const size_t& a, const size_t& b) { return (a == b); }

void add_example_namespace(example& ec, namespace_index ns, features& fs);
void del_example_namespace(example& ec, namespace_index ns, features& fs);

void set_label_features(label_feature_map& lfm, size_t lab, features& fs);

void add_example_namespaces_from_example(example& target, example& source);
void del_example_namespaces_from_example(example& target, example& source);
void add_example_namespace_from_memory(label_feature_map& lfm, example& ec, size_t lab);
void del_example_namespace_from_memory(label_feature_map& lfm, example& ec, size_t lab);

void free_label_features(label_feature_map& lfm);
}  // namespace LabelDict
