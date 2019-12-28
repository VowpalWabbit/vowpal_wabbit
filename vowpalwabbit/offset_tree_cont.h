#pragma once
#include "learner.h"
#include "options.h"

namespace VW { namespace offset_tree_cont {

  LEARNER::base_learner* offset_tree_cont_setup(config::options_i& options, vw& all);

  struct tree_node
  {
    tree_node(
        uint32_t node_id, uint32_t left_node_id, uint32_t right_node_id, uint32_t parent_id, uint32_t depth, bool is_leaf);

    inline bool operator==(const tree_node& rhs) const;
    bool operator!=(const tree_node& rhs) const;

    inline bool is_root() const { return id == parent_id; }

    uint32_t id;
    uint32_t left_id;
    uint32_t right_id;
    uint32_t parent_id;
    uint32_t depth;
    bool is_leaf;
  };

  struct min_depth_binary_tree
  {
    void build_tree(uint32_t num_nodes);
    inline uint32_t internal_node_count() const;
    inline uint32_t leaf_node_count() const;
    inline uint32_t depth() const;
    const tree_node& get_sibling(const tree_node& v);

    std::vector<tree_node> nodes;
    uint32_t root_idx = 0;

    private:
    uint32_t _num_leaf_nodes = 0;
    bool _initialized = false;
    uint32_t _depth = 0;
  };

  struct node_cost
  {
    uint32_t node_id;
    float cost;
  };

  bool compareByid(const node_cost& a, const node_cost& b);

  struct offset_tree
  {
    void init(uint32_t num_actions);
    int32_t learner_count() const;
    uint32_t predict(LEARNER::single_learner& base, example& ec);
    void init_node_sets(v_array<CB::cb_class>& ac);
    void reduce_depth();
    const tree_node& get_sibling(const tree_node& tree_node);
    void learn(LEARNER::single_learner& base, example& ec);
    void finish();

  private:
    min_depth_binary_tree _binary_tree;
    std::vector<node_cost> _nodes_depth;
    std::vector<node_cost> _nodes_depth_1;

    // Depth of reduction stack used to print debug statements with right indent
    uint32_t _dd = 0;   
  };
  }}
