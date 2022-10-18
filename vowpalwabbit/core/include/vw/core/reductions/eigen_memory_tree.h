// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once
#include "vw/core/array_parameters.h"
#include "vw/core/example.h"
#include "vw/core/vw_fwd.h"

#include <list>
#include <unordered_map>
#include <vector>

namespace VW
{
namespace reductions
{
VW::LEARNER::base_learner* eigen_memory_tree_setup(VW::setup_base_i& stack_builder);

namespace eigen_memory
{
struct tree_example
{
  VW::flat_example* base;  // base example only includes the base features without interaction flags
  VW::flat_example* full;  // full example includes the interactions that were passed in as flags
  uint32_t label;
  uint32_t tag;
  float score;

  tree_example();
  tree_example(VW::workspace& all, example* ex);
};

struct LRU
{
  typedef tree_example* K;
  typedef std::list<K>::iterator V;

  std::list<K> list;
  std::unordered_map<K, V> map;

  int max_size;

  LRU(int max_size);
  K bound(K item);
};

struct node
{
  node* parent;    // parent node
  bool internal;   // (internal or leaf)
  uint32_t depth;  // depth.
  node* left;      // left child.
  node* right;     // right child.

  std::vector<tree_example*> examples;

  double router_decision;
  sparse_parameters* router_weights;

  node();
};

struct tree
{
  VW::workspace* all;
  std::shared_ptr<VW::rand_state> _random_state;

  int iter;        // how many times we've 'learned'
  int depth;       // how deep the tree is
  int pass;        // what pass we are on for the data
  bool test_only;  // indicates that learning should not occur

  int32_t tree_bound;   // how many memories before bounding the tree
  int32_t leaf_split;   // how many memories before splitting a leaf node
  int32_t scorer_type;  // 1: random, 2: distance, 3: self-consistent rank, 4: not self-consistent rank
  int32_t router_type;  // 1: random approximation, 2: oja method

  example* ex;  // we create one of these which we re-use so we don't have to reallocate examples

  clock_t begin;  // for timing performance
  float time;     // for timing performance

  node* root;
  LRU* bounder;

  tree();
  void deallocate_node(node* n);
};

struct rng
{
  std::shared_ptr<VW::rand_state> state;

  rng(std::shared_ptr<VW::rand_state> state);

  typedef size_t result_type;
  static size_t min();
  static size_t max();
  size_t operator()();
};

}  // namespace eigen_memory
}  // namespace reductions
}  // namespace VW
