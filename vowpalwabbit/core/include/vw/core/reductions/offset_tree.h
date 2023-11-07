// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/core/learner_fwd.h"
#include "vw/core/vw_fwd.h"

#include <cstdint>
#include <memory>

namespace VW
{
namespace reductions
{
std::shared_ptr<VW::LEARNER::learner> offset_tree_setup(VW::setup_base_i& stack_builder);

namespace offset_tree
{
class tree_node
{
public:
  tree_node(uint32_t node_id, uint32_t left_node_id, uint32_t right_node_id, uint32_t parent_id, bool is_leaf);

  bool operator==(const tree_node& rhs) const;
  bool operator!=(const tree_node& rhs) const;

  uint32_t id;
  uint32_t left_id;
  uint32_t right_id;
  uint32_t parent_id;
  bool is_leaf;
};

class min_depth_binary_tree
{
public:
  void build_tree(uint32_t num_nodes);
  inline uint32_t internal_node_count() const;
  inline uint32_t leaf_node_count() const;

  std::vector<tree_node> nodes;
  uint32_t root_idx = 0;

private:
  uint32_t _num_leaf_nodes = 0;
  bool _initialized = false;
};

class offset_tree
{
public:
  using scores_t = std::vector<float>;
  using predict_buffer_t = std::vector<std::pair<float, float>>;

  offset_tree(uint32_t num_actions);
  void init();
  int32_t learner_count() const;
  const scores_t& predict(LEARNER::learner& base, example& ec);
  void learn(LEARNER::learner& base, example& ec);

private:
  min_depth_binary_tree binary_tree;
  uint32_t _num_actions = 0;
  predict_buffer_t _prediction_buffer{};
  std::vector<float> _scores{};
};
}  // namespace offset_tree
}  // namespace reductions
}  // namespace VW
