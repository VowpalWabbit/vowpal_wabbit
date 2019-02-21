#pragma once
#include <cstdint>
#include <vector>
#include "learner.h"
namespace offset_tree
{
  struct tree_node {
    tree_node(uint32_t i, uint32_t l, uint32_t r, bool lf);

    inline bool operator==(const tree_node& rhs) const;
    bool operator!=(const tree_node& rhs) const;

    uint32_t id;
    uint32_t left_id;
    uint32_t right_id;
    bool is_leaf;
  };

  struct min_depth_binary_tree {
    min_depth_binary_tree();
    void build_tree(uint32_t num_nodes);
    inline uint32_t internal_node_count() const;
    inline uint32_t leaf_node_count() const;

    std::vector< tree_node> nodes;
    uint32_t root_idx;
  private:
    uint32_t _num_leaf_nodes;
    bool _initialized;
  };

  struct offset_tree {
    void init(uint32_t num_actions);
    inline int32_t learner_count() const;
    const std::vector<float>& predict(LEARNER::single_learner& base, example& ec);
  private:
    min_depth_binary_tree binary_tree;
  };

}
