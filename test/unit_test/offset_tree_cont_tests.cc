#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>
#include <utility>
#include "offset_tree_cont.h"
using namespace LEARNER;
using namespace std;

namespace VW { namespace offset_tree_cont {
  std::ostream& operator<<(std::ostream& os, const tree_node& node) {
    os << "{" << node.id << "," << node.left_id << "," << node.right_id << ", " << node.parent_id << ", "
       << node.depth << ", " << (node.is_leaf ? "true" : "false")
       << "}";
    return os;
  }

struct reduction_test_harness {
  reduction_test_harness():
  _curr_idx(0){}

  void set_predict_response(const vector<float>& predictions) { _predictions = predictions; }

  void test_predict(single_learner& base, example& ec) {
   const auto curr_pred = _predictions[_curr_idx++];
   ec.pred.scalar = curr_pred;
  }

  void test_learn(single_learner& base, example& ec) {
    // do nothing
  }

  static void predict(reduction_test_harness& test_reduction, single_learner& base, example& ec) {
    test_reduction.test_predict(base,ec);
  }

  static void learn(reduction_test_harness& test_reduction, single_learner& base, example& ec) {
    test_reduction.test_learn(base,ec);
  };
private:
  vector<float> _predictions;
  int _curr_idx;
};

using test_learner_t = learner<reduction_test_harness, example>;
using predictions_t = vector<float>;
using scores_t = int;

void predict_test_helper(
    const predictions_t& base_reduction_predictions, const scores_t& expected_action, uint32_t num_leaves);

test_learner_t* get_test_harness_reduction(const predictions_t& base_reduction_predictions);
}  // namespace offset_tree_cont
}  // namespace VW

BOOST_AUTO_TEST_CASE(offset_tree_cont_learn_basic_1){
  // Setup a test harness base reduction
  const auto test_harness = VW::offset_tree_cont::get_test_harness_reduction({
      -1, 1
  });

  VW::offset_tree_cont::offset_tree tree;
  tree.init(3);
  example ec;
  ec.l.cb = CB::label();
  ec.l.cb.costs.push_back({2.0f, 1, 0.5f ,0.0f});
  ec.l.cb.costs.push_back({3.5f, 0, 0.5f, 0.0f});

  tree.learn(*as_singleline(test_harness), ec); 

  destroy_free<VW::offset_tree_cont::test_learner_t>(test_harness); 
}

BOOST_AUTO_TEST_CASE(offset_tree_cont_learn_basic_2)
{
  // Setup a test harness base reduction
  const auto test_harness = VW::offset_tree_cont::get_test_harness_reduction({1, 1});

  VW::offset_tree_cont::offset_tree tree;
  tree.init(3);
  example ec;
  ec.l.cb = CB::label();
  ec.l.cb.costs.push_back({2.0f, 1, 0.5f, 0.0f});
  ec.l.cb.costs.push_back({3.5f, 0, 0.5f, 0.0f});

  tree.learn(*as_singleline(test_harness), ec);

  destroy_free<VW::offset_tree_cont::test_learner_t>(test_harness);
}

BOOST_AUTO_TEST_CASE(offset_tree_cont_learn_basic_3)
{
  // Setup a test harness base reduction
  const auto test_harness = VW::offset_tree_cont::get_test_harness_reduction({1});

  VW::offset_tree_cont::offset_tree tree;
  tree.init(3);
  example ec;
  ec.l.cb = CB::label();
  ec.l.cb.costs.push_back({2.0f, 1, 0.5f, 0.0f});
  
  tree.learn(*as_singleline(test_harness), ec);

  destroy_free<VW::offset_tree_cont::test_learner_t>(test_harness);
}

BOOST_AUTO_TEST_CASE(offset_tree_cont_predict) {
  // 0 node tree
  VW::offset_tree_cont::predict_test_helper({}, 0, 0);  // todo: check this 
  // 1 node tree
  VW::offset_tree_cont::predict_test_helper({}, 1, 1); 
  // 2 node trees
  VW::offset_tree_cont::predict_test_helper({-1}, 1, 2);
  VW::offset_tree_cont::predict_test_helper({1}, 2, 2);
  // 3 node trees
  VW::offset_tree_cont::predict_test_helper({-1, 1}, 3, 3);
  VW::offset_tree_cont::predict_test_helper({-1, -1}, 2, 3);
  // 4 node tree
  VW::offset_tree_cont::predict_test_helper({-1, 1}, 2, 4);
  VW::offset_tree_cont::predict_test_helper({1, 1}, 4, 4);
  // 5 node tree
  VW::offset_tree_cont::predict_test_helper({-1, -1, 1}, 5, 5);
  VW::offset_tree_cont::predict_test_helper({1, 1}, 3, 5);
}

BOOST_AUTO_TEST_CASE(build_min_depth_tree_cont_5) {
  VW::offset_tree_cont::min_depth_binary_tree tree;
  tree.build_tree(5);
  std::vector<VW::offset_tree_cont::tree_node> expected = {
      {0, 1, 2, 0, 0, false},
      {1, 3, 4, 0, 1, false},
      {2, 5, 6, 0, 1, false},
      {3, 7, 8, 1, 2, false},
      {4, 0, 0, 1, 2, true},
      {5, 0, 0, 2, 2, true},
      {6, 0, 0, 2, 2, true},
      {7, 0, 0, 3, 3, true},
      {8, 0, 0, 3, 3, true}
  };
  BOOST_CHECK_EQUAL_COLLECTIONS(tree.nodes.begin(), tree.nodes.end(), expected.begin(), expected.end());
}

BOOST_AUTO_TEST_CASE(build_min_depth_tree_cont_4) {
  VW::offset_tree_cont::min_depth_binary_tree tree;
  tree.build_tree(4);
  std::vector<VW::offset_tree_cont::tree_node> expected = {
      {0, 1, 2, 0, 0, false},
      {1, 3, 4, 0, 1, false},
      {2, 5, 6, 0, 1, false},
      {3, 0, 0, 1, 2, true},
      {4, 0, 0, 1, 2, true},
      {5, 0, 0, 2, 2, true},
      {6, 0, 0, 2, 2, true}
  };
  BOOST_CHECK_EQUAL_COLLECTIONS(tree.nodes.begin(), tree.nodes.end(), expected.begin(), expected.end());
}

BOOST_AUTO_TEST_CASE(build_min_depth_tree_cont_3) {
  VW::offset_tree_cont::min_depth_binary_tree tree;
  tree.build_tree(3);
  std::vector<VW::offset_tree_cont::tree_node> expected = {
      {0, 1, 2, 0, 0, false},
      {1, 3, 4, 0, 1, false},
      {2, 0, 0, 0, 1, true},
      {3, 0, 0, 1, 2, true},
      {4, 0, 0, 1, 2, true},
  };
  BOOST_CHECK_EQUAL_COLLECTIONS(tree.nodes.begin(), tree.nodes.end(), expected.begin(), expected.end());
}

BOOST_AUTO_TEST_CASE(build_min_depth_tree_cont_1) {
  VW::offset_tree_cont::min_depth_binary_tree tree;
  tree.build_tree(1);
  std::vector<VW::offset_tree_cont::tree_node> expected = {
      {0, 0, 0, 0, 0, true}
  };
  BOOST_CHECK_EQUAL_COLLECTIONS(tree.nodes.begin(), tree.nodes.end(), expected.begin(), expected.end());
}

BOOST_AUTO_TEST_CASE(build_min_depth_tree_cont_too_big) {
  VW::offset_tree_cont::min_depth_binary_tree tree;
  // Throws vw_exception when unable to allocate enough memory to build tree
  BOOST_CHECK_THROW(tree.build_tree(INT_MAX),VW::vw_exception);
}

namespace VW { namespace offset_tree_cont {
test_learner_t* get_test_harness_reduction(const predictions_t& base_reduction_predictions)
{
  // Setup a test harness base reduction
  auto test_harness = scoped_calloc_or_throw<reduction_test_harness>();
  test_harness->set_predict_response(base_reduction_predictions);
  auto& test_learner =
      init_learner(test_harness,          // Data structure passed by vw_framework into test_harness predict/learn calls
          reduction_test_harness::learn,  // test_harness learn
          reduction_test_harness::predict,  // test_harness predict
          1                                 // Number of regressors in test_harness (not used)
      );                                    // Create a learner using the base reduction.
  return &test_learner;
}

void predict_test_helper(
    const predictions_t& base_reduction_predictions, const scores_t& expected_action, uint32_t num_leaves)
{
  const auto test_base = get_test_harness_reduction(base_reduction_predictions);
  VW::offset_tree_cont::offset_tree tree;
  tree.init(num_leaves);
  example ec;
  ec.l.cb.costs = v_init<CB::cb_class>();
  auto ret_val = tree.predict(*as_singleline(test_base), ec);
  BOOST_CHECK_EQUAL(ret_val, expected_action); //todo: check
  destroy_free<test_learner_t>(test_base);
}
}}
