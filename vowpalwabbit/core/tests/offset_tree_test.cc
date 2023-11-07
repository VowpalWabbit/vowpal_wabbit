// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/offset_tree.h"

#include "vw/core/learner.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <utility>

using namespace VW::LEARNER;
using namespace std;

using namespace ::testing;

namespace std
{
std::ostream& operator<<(std::ostream& os, const VW::reductions::offset_tree::tree_node& node)
{
  os << "{" << node.id << "," << node.left_id << "," << node.right_id << ", " << node.parent_id << ", "
     << (node.is_leaf ? "true" : "false") << "}";
  return os;
}
}  // namespace std

namespace
{
class reduction_test_harness
{
public:
  reduction_test_harness() : _curr_idx(0) {}

  void set_predict_response(const vector<pair<float, float>>& predictions) { _predictions = predictions; }

  void test_predict(VW::example& ec)
  {
    ec.pred.a_s.clear();
    const auto curr_pred = _predictions[_curr_idx++];
    ec.pred.a_s.push_back(VW::action_score{0, curr_pred.first});
    ec.pred.a_s.push_back(VW::action_score{1, curr_pred.second});
  }

  void test_learn(VW::example& /* ec */)
  {
    // do nothing
  }

  static void predict(reduction_test_harness& test_reduction, VW::example& ec) { test_reduction.test_predict(ec); }

  static void learn(reduction_test_harness& test_reduction, VW::example& ec) { test_reduction.test_learn(ec); };

private:
  vector<pair<float, float>> _predictions;
  int _curr_idx;
};

using predictions_t = vector<pair<float, float>>;
using scores_t = std::vector<float>;

std::shared_ptr<learner> get_test_harness(const predictions_t& bottom_learner_predictions)
{
  // Setup a test harness bottom learner
  auto test_harness = VW::make_unique<reduction_test_harness>();
  test_harness->set_predict_response(bottom_learner_predictions);
  auto test_learner = VW::LEARNER::make_bottom_learner(
      std::move(test_harness),          // Data structure passed by vw_framework into test_harness predict/learn calls
      reduction_test_harness::learn,    // test_harness learn
      reduction_test_harness::predict,  // test_harness predict
      "test_learner", VW::prediction_type_t::ACTION_SCORES, VW::label_type_t::CB)
                          .set_output_example_prediction([](VW::workspace& /* all */, const reduction_test_harness&,
                                                             const VW::example&, VW::io::logger&) {})

                          .build();  // Create a learner using the bottom learner.
  return test_learner;
}

void predict_test_helper(const predictions_t& bottom_learner_predictions, const scores_t& expected_scores)
{
  const auto test_base = get_test_harness(bottom_learner_predictions);
  VW::reductions::offset_tree::offset_tree tree(static_cast<uint32_t>(expected_scores.size()));
  tree.init();
  VW::example ec;
  auto& ret_val = tree.predict(*test_base, ec);
  EXPECT_THAT(ret_val, ContainerEq(expected_scores));
}
}  // namespace

TEST(OffsetTree, OffsetTreeLearnBasic)
{
  // Setup a test harness bottom learner
  const auto test_harness = get_test_harness({{.9f, .1f}, {.9f, .1f}});

  VW::reductions::offset_tree::offset_tree tree(3);
  tree.init();
  VW::example ec;
  ec.l.cb.costs.push_back(VW::cb_class{-1.0f, 1, 0.5f});

  tree.learn(*test_harness, ec);
}

TEST(OffsetTree, OffsetTreePredict)
{
  // 0 node tree
  predict_test_helper({{}}, {});
  // 1 node tree
  predict_test_helper({{}}, {1.0f});
  // 2 node trees
  predict_test_helper({{.2f, .8f}}, {.2f, .8f});
  predict_test_helper({{.1f, .9f}}, {.1f, .9f});
  predict_test_helper({{0.0f, 1.0f}}, {0.0f, 1.0f});
  // 3 node trees
  predict_test_helper({{.9f, .1f}, {.9f, .1f}}, {.9f * .9f, .9f * .1f, .1f});
  predict_test_helper({{.2f, .8f}, {.2f, .8f}}, {.2f * .2f, .2f * .8f, .8f});
  // 4 node tree
  predict_test_helper({{.9f, .1f}, {.9f, .1f}, {.9f, .1f}}, {.9f * .9f, .9f * .1f, .1f * .9f, .1f * .1f});
  // 5 node tree
  predict_test_helper({{.9f, .1f}, {.9f, .1f}, {.9f, .1f}, {.9f, .1f}},
      {.9f * .9f * .9f, .9f * .9f * .1f, .9f * .1f * .9f, .9f * .1f * .1f, .1f});
}

TEST(OffsetTree, BuildMinDepthTree11)
{
  VW::reductions::offset_tree::min_depth_binary_tree tree;
  tree.build_tree(11);
  std::vector<VW::reductions::offset_tree::tree_node> expected = {{0, 0, 0, 11, true}, {1, 0, 0, 11, true},
      {2, 0, 0, 12, true}, {3, 0, 0, 12, true}, {4, 0, 0, 13, true}, {5, 0, 0, 13, true}, {6, 0, 0, 14, true},
      {7, 0, 0, 14, true}, {8, 0, 0, 15, true}, {9, 0, 0, 15, true}, {10, 0, 0, 18, true}, {11, 0, 1, 16, false},
      {12, 2, 3, 16, false}, {13, 4, 5, 17, false}, {14, 6, 7, 17, false}, {15, 8, 9, 18, false},
      {16, 11, 12, 19, false}, {17, 13, 14, 19, false}, {18, 15, 10, 20, false}, {19, 16, 17, 20, false},
      {20, 19, 18, 20, false}};
  EXPECT_THAT(tree.nodes, ContainerEq(expected));
}

TEST(OffsetTree, BuildMinDepthTree4)
{
  VW::reductions::offset_tree::min_depth_binary_tree tree;
  tree.build_tree(4);
  std::vector<VW::reductions::offset_tree::tree_node> expected = {{0, 0, 0, 4, true}, {1, 0, 0, 4, true},
      {2, 0, 0, 5, true}, {3, 0, 0, 5, true}, {4, 0, 1, 6, false}, {5, 2, 3, 6, false}, {6, 4, 5, 6, false}};
  EXPECT_THAT(tree.nodes, ContainerEq(expected));
}

TEST(OffsetTree, BuildMinDepthTree3)
{
  VW::reductions::offset_tree::min_depth_binary_tree tree;
  tree.build_tree(3);
  std::vector<VW::reductions::offset_tree::tree_node> expected = {
      {0, 0, 0, 3, true}, {1, 0, 0, 3, true}, {2, 0, 0, 4, true}, {3, 0, 1, 4, false}, {4, 3, 2, 4, false}};
  EXPECT_THAT(tree.nodes, ContainerEq(expected));
}

TEST(OffsetTree, BuildMinDepthTree1)
{
  VW::reductions::offset_tree::min_depth_binary_tree tree;
  tree.build_tree(1);
  std::vector<VW::reductions::offset_tree::tree_node> expected = {{0, 0, 0, 0, true}};
  EXPECT_THAT(tree.nodes, ContainerEq(expected));
}
