#include "offset_tree_internal.h"
#include "vw_exception.h"

namespace offset_tree {
  tree_node::tree_node(uint32_t i, uint32_t l, uint32_t r, bool lf)
    : id(i), left_id(l), right_id(r), is_leaf(lf) {}

  bool tree_node::operator==(const tree_node& rhs) const {
    if (this == &rhs) return true;
    return (id == rhs.id && left_id == rhs.left_id && right_id == rhs.right_id && is_leaf == rhs.is_leaf);
  }

  bool tree_node::operator!=(const tree_node& rhs) const { return !(*this == rhs); }

  min_depth_binary_tree::min_depth_binary_tree()
    : root_idx{0}, _num_leaf_nodes{0}, _initialized{false} {}

  void min_depth_binary_tree::build_tree(uint32_t num_nodes) {
    // Sanity checks
    if (_initialized) {
      if (num_nodes != _num_leaf_nodes) {
        THROW("Tree already initialized.  New leaf node count (" << num_nodes << ") does not equal current value. (" << _num_leaf_nodes << ")");
      }
      return;
    }

    _num_leaf_nodes = num_nodes;
    // deal with degenerate cases of 0 and 1 actions
    if(_num_leaf_nodes == 0) {
      _initialized = true;
      return;
    }

    try {
      nodes.reserve(2 * _num_leaf_nodes - 1);
      std::vector<uint32_t> tournaments;
      tournaments.reserve(_num_leaf_nodes); // Add all leaf nodes to node collection
      for (uint32_t i = 0; i < _num_leaf_nodes; i++) {
        nodes.emplace_back(i, 0, 0, true);
        tournaments.emplace_back(i);
      }
      while (tournaments.size() > 1) {
        const auto num_tournaments = tournaments.size(); // ids of next set of nodes start with the current
        // node count (since ids are zero based)
        auto id = nodes.size();
        std::vector<uint32_t> new_tournaments;
        new_tournaments.reserve(num_tournaments / 2 + 1);
        for (size_t j = 0; j < num_tournaments / 2; ++j) {
          auto left = tournaments[2 * j];
          auto right = tournaments[2 * j + 1];
          new_tournaments.emplace_back(id);
          nodes.emplace_back(id++, left, right, false);
        }
        if (num_tournaments % 2 == 1) new_tournaments.emplace_back(tournaments.back());
        tournaments = std::move(new_tournaments);
      }
      root_idx = tournaments[0];
    }
    catch (std::bad_alloc& e) {
      THROW("Unable to allocate memory for offset tree.  Label count:" << _num_leaf_nodes << " bad_alloc:" << e.what());
    }
    _initialized = true;
  }

  uint32_t min_depth_binary_tree::internal_node_count() const {
    return nodes.size() - _num_leaf_nodes;
  }

  uint32_t min_depth_binary_tree::leaf_node_count() const {
    return _num_leaf_nodes;
  }

  void offset_tree::init(uint32_t num_actions) {
    binary_tree.build_tree(num_actions);
  }

  int32_t offset_tree::learner_count() const {
    return binary_tree.internal_node_count();
  }

  const std::vector<float>& offset_tree::predict(LEARNER::single_learner& base, example& ec) {
    static thread_local std::vector<std::pair<float,float>> prediction_buffer(learner_count());
    static thread_local std::vector<float> scores(binary_tree.leaf_node_count());

    auto& t = binary_tree;

    prediction_buffer.clear();
    scores.resize(t.leaf_node_count());

    // Handle degenerate cases of zero and one node trees
    if (t.leaf_node_count() == 0)
      return scores;
    if (t.leaf_node_count() == 1) {
      scores[0] = 1.0f;
      return scores;
    }

    // Get predictions for all internal nodes
    for (auto idx = 0; idx < t.internal_node_count(); ++idx) {
      base.predict(ec, idx);
      prediction_buffer.emplace_back(ec.pred.a_s[0].score, ec.pred.a_s[1].score);
    }

    //Compute action scores
    const auto lrnr_strt_offset = t.leaf_node_count();
    for (auto rit = t.nodes.rbegin(); rit != t.nodes.rend(); ++rit) {

      if (rit->is_leaf) // done processing all internal nodes
        break;

      // update probabilities for left node
      const float left_p = prediction_buffer[rit->id - lrnr_strt_offset].first;
      if(t.nodes[rit->left_id].is_leaf) {
        scores[rit->left_id] = left_p;
      }
      else {
        const auto left_left_p = prediction_buffer[rit->left_id - lrnr_strt_offset].first;
        prediction_buffer[rit->left_id - lrnr_strt_offset].first = left_left_p * left_p;
        const auto left_right_p = prediction_buffer[rit->left_id - lrnr_strt_offset].second;
        prediction_buffer[rit->left_id - lrnr_strt_offset].second = left_right_p * left_p;
      }

      // update probabilities for right node
      const float right_p = prediction_buffer[rit->id - lrnr_strt_offset].second;
      if (t.nodes[rit->right_id].is_leaf) {
        scores[rit->right_id] = right_p;
      }
      else {
        const auto right_left_p = prediction_buffer[rit->right_id - lrnr_strt_offset].first;
        prediction_buffer[rit->right_id - lrnr_strt_offset].first = right_left_p * right_p;
        const auto right_right_p = prediction_buffer[rit->right_id - lrnr_strt_offset].second;
        prediction_buffer[rit->right_id - lrnr_strt_offset].second = right_right_p * right_p;
      }
    }

    return scores;
  }
}
