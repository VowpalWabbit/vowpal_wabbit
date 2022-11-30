// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once
#include "vw/core/example.h"

#include <list>
#include <unordered_map>
#include <vector>

namespace VW
{
namespace reductions
{
VW::LEARNER::base_learner* eigen_memory_tree_setup(VW::setup_base_i& stack_builder);

namespace eigen_memory_tree
{
using emt_feats = std::vector<std::pair<int, float>>;

enum class emt_scorer_type
{
  random = 1,
  distance = 2,
  self_consistent_rank = 3,
  not_self_consistent_rank = 4
};

enum class emt_router_type
{
  random = 1,
  eigen = 2
};

float emt_median(std::vector<float>&);

float emt_inner(const emt_feats&, const emt_feats&);

float emt_norm(const emt_feats&);

void emt_abs(emt_feats&);

void emt_scale(emt_feats&, float);

void emt_normalize(emt_feats&);

emt_feats emt_scale_add(float, const emt_feats&, float, const emt_feats&);

emt_feats emt_router_eigen(std::vector<emt_feats>&, VW::rand_state&);

struct emt_example
{
  emt_feats base;  // base example only includes the base features without interaction flags
  emt_feats full;  // full example includes the interactions that were passed in as flags
  uint32_t label;

  emt_example();
  emt_example(VW::workspace&, VW::example*);
};

struct emt_lru
{
  using K = emt_example*;
  using V = std::list<K>::iterator;

  std::list<K> list;
  std::unordered_map<K, V> map;

  int max_size;

  emt_lru(unsigned long);
  K bound(K);
};

struct emt_node
{
  double router_decision;
  std::unique_ptr<emt_node> left;
  std::unique_ptr<emt_node> right;
  emt_feats router_weights;

  std::vector<std::unique_ptr<emt_example>> examples;

  emt_node();
};

struct emt_tree
{
  VW::workspace* all;
  std::shared_ptr<VW::rand_state> _random_state;

  uint32_t leaf_split;  // how many memories before splitting a leaf node
  emt_scorer_type scorer_type;
  emt_router_type router_type;

  std::unique_ptr<VW::example> ex;  // we create one of these which we re-use so we don't have to reallocate examples

  long begin;  // for timing performance

  std::unique_ptr<emt_node> root;
  std::unique_ptr<emt_lru> bounder;

  emt_tree();
  ~emt_tree();
};

}  // namespace eigen_memory_tree
}  // namespace reductions
}  // namespace VW
