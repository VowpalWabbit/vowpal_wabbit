// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once
#include "vw/core/array_parameters.h"
#include "vw/core/example.h"
#include "vw/core/vw_fwd.h"

#include <ctime>
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

enum class emt_scorer_feature_type
{
  abs_diff,
  diff
};

enum class emt_scorer_example_type
{
  self_consistent,
  not_self_consistent
};


struct emt_example
{
  VW::flat_example* base;  // base example only includes the base features without interaction flags
  VW::flat_example* full;  // full example includes the interactions that were passed in as flags
  uint32_t label;
  uint32_t tag;
  float score;

  emt_example();
  emt_example(VW::workspace& all, VW::example* ex);
};

struct emt_lru
{
  using K = emt_example*;
  using V = std::list<K>::iterator;

  std::list<K> list;
  std::unordered_map<K, V> map;

  int max_size;

  emt_lru(unsigned long max_size);
  K bound(K item);
};

struct emt_node
{
  emt_node* parent; // parent node
  bool internal;    // (internal or leaf)
  uint32_t depth;   // depth.
  emt_node* left;   // left child.
  emt_node* right;  // right child.

  std::vector<emt_example*> examples;

  double router_decision;
  sparse_parameters* router_weights;

  emt_node();
};

struct emt_tree
{
  VW::workspace* all;
  std::shared_ptr<VW::rand_state> _random_state;

  uint32_t iter;   // how many times we've 'learned'
  uint32_t depth;  // how deep the tree is
  uint32_t pass;   // what pass we are on for the data
  bool test_only;  // indicates that learning should not occur

  int32_t tree_bound;   // how many memories before bounding the tree
  uint32_t leaf_split;  // how many memories before splitting a leaf node
  emt_scorer_type scorer_type;
  emt_router_type router_type;

  VW::example* ex;  // we create one of these which we re-use so we don't have to reallocate examples

  std::clock_t begin;  // for timing performance
  float time;          // for timing performance

  emt_node* root;
  emt_lru* bounder;

  emt_tree();
  void deallocate_node(emt_node* n);
  ~emt_tree();
};

}  // namespace eigen_memory
}  // namespace reductions
}  // namespace VW
