#pragma once
#include <cstdint>
#include <vector>
#include "learner.h"

namespace VW { namespace offset_tree {

  struct tree_node {
    tree_node(uint32_t node_id, uint32_t left_node_id, uint32_t right_node_id, bool is_leaf);

    inline bool operator==(const tree_node& rhs) const;
    bool operator!=(const tree_node& rhs) const;

    uint32_t id;
    uint32_t left_id;
    uint32_t right_id;
    bool is_leaf;
  };

  struct min_depth_binary_tree {
    void build_tree(uint32_t num_nodes);
    inline uint32_t internal_node_count() const;
    inline uint32_t leaf_node_count() const;

    std::vector<tree_node> nodes;
    uint32_t root_idx = 0;
  private:
    uint32_t _num_leaf_nodes = 0;
    bool _initialized = false;
  };

}}
