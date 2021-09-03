#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>
#include <utility>
#include "offset_tree.h"
#include "test_common.h"
#include "cb_label_parser.h"

using namespace VW::LEARNER;
using namespace std;

namespace VW
{
namespace offset_tree
{
std::ostream& operator<<(std::ostream& os, const tree_node& node)
{
  os << "{" << node.id << "," << node.left_id << "," << node.right_id << ", " << node.parent_id << ", "
     << (node.is_leaf ? "true" : "false") << "}";
  return os;
}

struct reduction_test_harness
{
  reduction_test_harness() : _curr_idx(0) {}

  void set_predict_response(const vector<pair<float, float>>& predictions) { _predictions = predictions; }

  void test_predict(base_learner& base, example& ec)
  {
    ec.pred.a_s.clear();
    const auto curr_pred = _predictions[_curr_idx++];
    ec.pred.a_s.push_back(ACTION_SCORE::action_score{0, curr_pred.first});
    ec.pred.a_s.push_back(ACTION_SCORE::action_score{1, curr_pred.second});
  }

  void test_learn(base_learner& base, example& ec)
  {
    // do nothing
  }

  static void predict(reduction_test_harness& test_reduction, base_learner& base, example& ec)
  {
    test_reduction.test_predict(base, ec);
  }

  static void learn(reduction_test_harness& test_reduction, base_learner& base, example& ec)
  {
    test_reduction.test_learn(base, ec);
  };

private:
  vector<pair<float, float>> _predictions;
  int _curr_idx;
};

using test_learner_t = learner<reduction_test_harness, example>;
using predictions_t = vector<pair<float, float>>;
using scores_t = std::vector<float>;

void predict_test_helper(const predictions_t& base_reduction_predictions, const scores_t& expected_scores);
test_learner_t* get_test_harness_reduction(const predictions_t& base_reduction_predictions);
}  // namespace offset_tree
}  // namespace VW

BOOST_AUTO_TEST_CASE(offset_tree_learn_basic)
{
  // Setup a test harness base reduction
  const auto test_harness = VW::offset_tree::get_test_harness_reduction({{.9f, .1f}, {.9f, .1f}});

  VW::offset_tree::offset_tree tree(3);
  tree.init();
  example ec;
  ec.l.cb.costs.push_back(CB::cb_class{-1.0f, 1, 0.5f});

  tree.learn(*as_singleline(test_harness), ec);
  delete test_harness;
}

BOOST_AUTO_TEST_CASE(offset_tree_predict)
{
  // 0 node tree
  VW::offset_tree::predict_test_helper({{}}, {});
  // 1 node tree
  VW::offset_tree::predict_test_helper({{}}, {1.0f});
  // 2 node trees
  VW::offset_tree::predict_test_helper({{.2f, .8f}}, {.2f, .8f});
  VW::offset_tree::predict_test_helper({{.1f, .9f}}, {.1f, .9f});
  VW::offset_tree::predict_test_helper({{0.0f, 1.0f}}, {0.0f, 1.0f});
  // 3 node trees
  VW::offset_tree::predict_test_helper({{.9f, .1f}, {.9f, .1f}}, {.9f * .9f, .9f * .1f, .1f});
  VW::offset_tree::predict_test_helper({{.2f, .8f}, {.2f, .8f}}, {.2f * .2f, .2f * .8f, .8f});
  // 4 node tree
  VW::offset_tree::predict_test_helper(
      {{.9f, .1f}, {.9f, .1f}, {.9f, .1f}}, {.9f * .9f, .9f * .1f, .1f * .9f, .1f * .1f});
  // 5 node tree
  VW::offset_tree::predict_test_helper({{.9f, .1f}, {.9f, .1f}, {.9f, .1f}, {.9f, .1f}},
      {.9f * .9f * .9f, .9f * .9f * .1f, .9f * .1f * .9f, .9f * .1f * .1f, .1f});
}

BOOST_AUTO_TEST_CASE(build_min_depth_tree_11)
{
  VW::offset_tree::min_depth_binary_tree tree;
  tree.build_tree(11);
  std::vector<VW::offset_tree::tree_node> expected = {{0, 0, 0, 11, true}, {1, 0, 0, 11, true}, {2, 0, 0, 12, true},
      {3, 0, 0, 12, true}, {4, 0, 0, 13, true}, {5, 0, 0, 13, true}, {6, 0, 0, 14, true}, {7, 0, 0, 14, true},
      {8, 0, 0, 15, true}, {9, 0, 0, 15, true}, {10, 0, 0, 18, true}, {11, 0, 1, 16, false}, {12, 2, 3, 16, false},
      {13, 4, 5, 17, false}, {14, 6, 7, 17, false}, {15, 8, 9, 18, false}, {16, 11, 12, 19, false},
      {17, 13, 14, 19, false}, {18, 15, 10, 20, false}, {19, 16, 17, 20, false}, {20, 19, 18, 20, false}};
  BOOST_CHECK_EQUAL_COLLECTIONS(tree.nodes.begin(), tree.nodes.end(), expected.begin(), expected.end());
}

BOOST_AUTO_TEST_CASE(build_min_depth_tree_4)
{
  VW::offset_tree::min_depth_binary_tree tree;
  tree.build_tree(4);
  std::vector<VW::offset_tree::tree_node> expected = {{0, 0, 0, 4, true}, {1, 0, 0, 4, true}, {2, 0, 0, 5, true},
      {3, 0, 0, 5, true}, {4, 0, 1, 6, false}, {5, 2, 3, 6, false}, {6, 4, 5, 6, false}};
  BOOST_CHECK_EQUAL_COLLECTIONS(tree.nodes.begin(), tree.nodes.end(), expected.begin(), expected.end());
}

BOOST_AUTO_TEST_CASE(build_min_depth_tree_3)
{
  VW::offset_tree::min_depth_binary_tree tree;
  tree.build_tree(3);
  std::vector<VW::offset_tree::tree_node> expected = {
      {0, 0, 0, 3, true}, {1, 0, 0, 3, true}, {2, 0, 0, 4, true}, {3, 0, 1, 4, false}, {4, 3, 2, 4, false}};
  BOOST_CHECK_EQUAL_COLLECTIONS(tree.nodes.begin(), tree.nodes.end(), expected.begin(), expected.end());
}

BOOST_AUTO_TEST_CASE(build_min_depth_tree_1)
{
  VW::offset_tree::min_depth_binary_tree tree;
  tree.build_tree(1);
  std::vector<VW::offset_tree::tree_node> expected = {{0, 0, 0, 0, true}};
  BOOST_CHECK_EQUAL_COLLECTIONS(tree.nodes.begin(), tree.nodes.end(), expected.begin(), expected.end());
}

namespace VW
{
namespace offset_tree
{
test_learner_t* get_test_harness_reduction(const predictions_t& base_reduction_predictions)
{
  // Setup a test harness base reduction
  auto test_harness = VW::make_unique<reduction_test_harness>();
  test_harness->set_predict_response(base_reduction_predictions);
  auto test_learner = VW::LEARNER::make_base_learner(
      std::move(test_harness),          // Data structure passed by vw_framework into test_harness predict/learn calls
      reduction_test_harness::learn,    // test_harness learn
      reduction_test_harness::predict,  // test_harness predict
      "test_learner", prediction_type_t::action_scores, label_type_t::cb)
                          .build();  // Create a learner using the base reduction.
  return test_learner;
}

void predict_test_helper(const predictions_t& base_reduction_predictions, const scores_t& expected_scores)
{
  const auto test_base = get_test_harness_reduction(base_reduction_predictions);
  VW::offset_tree::offset_tree tree(static_cast<uint32_t>(expected_scores.size()));
  tree.init();
  example ec;
  auto& ret_val = tree.predict(*as_singleline(test_base), ec);
  BOOST_CHECK_EQUAL_COLLECTIONS(ret_val.begin(), ret_val.end(), expected_scores.begin(), expected_scores.end());
  delete test_base;
}

}  // namespace offset_tree
}  // namespace VW
