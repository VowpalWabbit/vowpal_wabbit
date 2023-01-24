// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/common/hash.h"
#include "vw/core/cb.h"
#include "vw/core/learner_fwd.h"
#include "vw/core/vw_fwd.h"

#include <memory>
#include <vector>

namespace VW
{
namespace reductions
{
std::shared_ptr<VW::LEARNER::learner> cats_tree_setup(setup_base_i& stack_builder);

namespace cats
{
class tree_node
{
public:
  tree_node(uint32_t node_id, uint32_t left_node_id, uint32_t right_node_id, uint32_t parent_id, uint32_t depth,
      bool left_only, bool right_only, bool is_leaf);

  bool operator==(const tree_node& rhs) const;
  bool operator!=(const tree_node& rhs) const;

  inline bool is_root() const { return id == parent_id; }

  uint32_t id;
  uint32_t left_id;
  uint32_t right_id;
  uint32_t parent_id;
  uint32_t depth;
  bool left_only;
  bool right_only;
  bool is_leaf;
  uint32_t learn_count;
};

class min_depth_binary_tree
{
public:
  void build_tree(uint32_t num_nodes, uint32_t bandwidth);
  inline uint32_t internal_node_count() const;
  inline uint32_t leaf_node_count() const;
  inline uint32_t depth() const;
  const tree_node& get_sibling(const tree_node& v);
  std::string tree_stats_to_string();
  std::vector<tree_node> nodes;
  uint32_t root_idx = 0;

private:
  uint32_t _num_leaf_nodes = 0;
  bool _initialized = false;
  uint32_t _depth = 0;
};

class node_cost
{
public:
  uint32_t node_id;
  float cost;
  node_cost() : node_id(0), cost(0.f) {}
  node_cost(uint32_t node_id, float cost) : node_id(node_id), cost(cost) {}
};

class cats_tree
{
public:
  void init(uint32_t num_actions, uint32_t bandwidth);
  int32_t learner_count() const;
  uint32_t predict(LEARNER::learner& base, example& ec);
  void init_node_costs(std::vector<VW::cb_class>& ac);
  const tree_node& get_sibling(const tree_node& tree_node);
  float return_cost(const tree_node& w);
  void learn(LEARNER::learner& base, example& ec);
  void set_trace_message(std::shared_ptr<std::ostream> ostrm, bool quiet);
  ~cats_tree();

private:
  uint64_t app_seed = VW::uniform_hash("vw", 2, 0);
  std::string tree_stats_to_string();
  min_depth_binary_tree _binary_tree;
  float _cost_star = 0.f;
  node_cost _a;
  node_cost _b;
  std::shared_ptr<std::ostream> _trace_stream = nullptr;
  bool _quiet = false;
};

}  // namespace cats
}  // namespace reductions
}  // namespace VW