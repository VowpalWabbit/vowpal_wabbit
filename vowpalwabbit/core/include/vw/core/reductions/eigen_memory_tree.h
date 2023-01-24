// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/common/random.h"
#include "vw/common/string_view.h"
#include "vw/core/feature_group.h"
#include "vw/core/vw_fwd.h"

#include <list>
#include <memory>
#include <unordered_map>
#include <vector>

// Uncommenting this enables a timer that prints the pass time at the end of each pass.
// #define VW_ENABLE_EMT_DEBUG_TIMER

namespace VW
{
namespace reductions
{
std::shared_ptr<VW::LEARNER::learner> eigen_memory_tree_setup(VW::setup_base_i& stack_builder);

namespace eigen_memory_tree
{
using emt_feats = std::vector<std::pair<uint64_t, float>>;

enum class emt_scorer_type : uint32_t
{
  RANDOM = 1,
  DISTANCE = 2,
  SELF_CONSISTENT_RANK = 3,
  NOT_SELF_CONSISTENT_RANK = 4
};

enum class emt_router_type : uint32_t
{
  RANDOM = 1,
  EIGEN = 2
};

emt_scorer_type emt_scorer_type_from_string(VW::string_view val);
emt_router_type emt_router_type_from_string(VW::string_view val);

float emt_median(std::vector<float>&);
float emt_inner(const emt_feats&, const emt_feats&);
float emt_norm(const emt_feats&);
void emt_abs(emt_feats&);
void emt_scale(emt_feats&, float);
void emt_normalize(emt_feats&);
emt_feats emt_scale_add(float, const emt_feats&, float, const emt_feats&);
emt_feats emt_router_eigen(std::vector<emt_feats>&, VW::rand_state&);

template <typename RandomIt>
void emt_shuffle(RandomIt first, RandomIt last, VW::rand_state& rng)
{
  // This is Richard Durstenfeld's method popularized by Donald Knuth in The Art of Computer Programming.
  // This algorithm is unbiased (i.e., all possible permutations are equally likely to occur).

  auto n = std::distance(first, last);
  if (n < 2) { return; }

  for (int i = 0; i < n - 1; i++)
  {
    int j = static_cast<int>(rng.get_and_update_random() * .999 * (n - i));
    std::iter_swap(first + i, first + j);
  }
}

struct emt_example
{
  emt_feats base;  // base example only includes the base features without interaction flags
  emt_feats full;  // full example includes the interactions that were passed in as flags
  uint32_t label = 0;

  emt_example() = default;
  emt_example(VW::workspace&, VW::example*);
};

struct emt_lru
{
  using K = emt_example*;
  using V = std::list<K>::iterator;

  std::list<K> list;
  std::unordered_map<K, V> map;

  uint64_t max_size;

  emt_lru(uint64_t);
  K bound(K);
};

struct emt_node
{
  double router_decision = 0;
  std::unique_ptr<emt_node> left = nullptr;
  std::unique_ptr<emt_node> right = nullptr;
  emt_feats router_weights;

  std::vector<std::unique_ptr<emt_example>> examples;
};

struct emt_tree
{
  VW::workspace* all = nullptr;
  std::shared_ptr<VW::rand_state> random_state;

  // how many memories before splitting a leaf node
  uint32_t leaf_split = 100;
  emt_scorer_type scorer_type = emt_scorer_type::SELF_CONSISTENT_RANK;
  emt_router_type router_type = emt_router_type::EIGEN;

  std::unique_ptr<VW::example> ex;  // we create one of these which we re-use so we don't have to reallocate examples
  std::unique_ptr<std::vector<std::vector<VW::namespace_index>>> empty_interactions_for_ex;
  std::unique_ptr<std::vector<std::vector<extent_term>>> empty_extent_interactions_for_ex;

#ifdef VW_ENABLE_EMT_DEBUG_TIMER
  int64_t begin = 0;  // for timing performance
#endif

  std::unique_ptr<emt_node> root;
  std::unique_ptr<emt_lru> bounder = nullptr;

  emt_tree(VW::workspace* all, std::shared_ptr<VW::rand_state> random_state, uint32_t leaf_split,
      emt_scorer_type scorer_type, emt_router_type router_type, uint64_t tree_bound);
};

}  // namespace eigen_memory_tree
}  // namespace reductions

namespace model_utils
{
size_t read_model_field(io_buf& io, reductions::eigen_memory_tree::emt_example& ex);
size_t write_model_field(
    io_buf& io, const reductions::eigen_memory_tree::emt_example& ex, const std::string& upstream_name, bool text);
size_t read_model_field(io_buf& io, reductions::eigen_memory_tree::emt_node& node);
size_t write_model_field(
    io_buf& io, const reductions::eigen_memory_tree::emt_node& node, const std::string& upstream_name, bool text);
size_t read_model_field(io_buf& io, reductions::eigen_memory_tree::emt_tree& tree);
size_t write_model_field(
    io_buf& io, const reductions::eigen_memory_tree::emt_tree& tree, const std::string& upstream_name, bool text);
}  // namespace model_utils
}  // namespace VW
