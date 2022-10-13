// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/cats_tree.h"

#include "test_common.h"
#include "vw/core/cb_label_parser.h"
#include "vw/core/simple_label.h"

#include <boost/test/unit_test.hpp>

using namespace VW::LEARNER;
using std::vector;

namespace std
{
std::ostream& operator<<(std::ostream& os, const VW::reductions::cats::tree_node& node)
{
  os << "{" << node.id << "," << node.left_id << "," << node.right_id << ", " << node.parent_id << ", " << node.depth
     << ", " << (node.is_leaf ? "true" : "false") << "}";
  return os;
}
}  // namespace std

namespace VW
{
std::ostream& operator<<(std::ostream& o, VW::simple_label const& lbl)
{
  o << "{l=" << lbl.label << "}";
  return o;
}

std::ostream& operator<<(std::ostream& o, VW::simple_label_reduction_features const& red_fts)
{
  o << "{w=" << red_fts.weight << ", i=" << red_fts.initial << "}";
  return o;
}
}  // namespace VW

class reduction_test_harness
{
public:
  reduction_test_harness() : _curr_idx(0) {}

  void set_predict_response(const vector<float>& predictions) { _predictions = predictions; }

  void test_predict(base_learner& base, VW::example& ec) { ec.pred.scalar = _predictions[_curr_idx++]; }

  void test_learn(base_learner& base, VW::example& ec)
  {
    _labels.emplace_back(ec.l.simple);
    _weights.emplace_back(ec.weight);
    _learner_offset.emplace_back(ec.ft_offset);
  }

  // use NO_SANITIZE_UNDEFINED because reference base_learner& base may be bound to nullptr
  static void NO_SANITIZE_UNDEFINED predict(reduction_test_harness& test_reduction, base_learner& base, VW::example& ec)
  {
    test_reduction.test_predict(base, ec);
  }

  static void NO_SANITIZE_UNDEFINED learn(reduction_test_harness& test_reduction, base_learner& base, VW::example& ec)
  {
    test_reduction.test_learn(base, ec);
  };

  vector<float> _predictions;
  vector<VW::simple_label> _labels;
  vector<float> _weights;
  vector<uint64_t> _learner_offset;
  int _curr_idx;
};

using test_learner_t = learner<reduction_test_harness, VW::example>;
using predictions_t = vector<float>;
using scores_t = int;

template <typename T = reduction_test_harness>
learner<T, VW::example>* get_test_harness_reduction(const predictions_t& base_reduction_predictions)
{
  T* pharness = nullptr;
  return get_test_harness_reduction(base_reduction_predictions, pharness);
}

template <typename T = reduction_test_harness>
learner<T, VW::example>* get_test_harness_reduction(const predictions_t& base_reduction_predictions, T*& pharness)
{
  // Setup a test harness base reduction
  auto test_harness = VW::make_unique<T>();
  pharness = test_harness.get();
  test_harness->set_predict_response(base_reduction_predictions);
  auto test_learner = VW::LEARNER::make_base_learner(
      std::move(test_harness),          // Data structure passed by vw_framework into test_harness predict/learn calls
      reduction_test_harness::learn,    // test_harness learn
      reduction_test_harness::predict,  // test_harness predict
      "test_learner", VW::prediction_type_t::scalar, VW::label_type_t::cb)
                          .build();  // Create a learner using the base reduction.
  return test_learner;
}

void predict_test_helper(const predictions_t& base_reduction_predictions, const scores_t& expected_action,
    uint32_t num_leaves, uint32_t bandwidth)
{
  const auto test_base = get_test_harness_reduction(base_reduction_predictions);
  VW::reductions::cats::cats_tree tree;
  tree.init(num_leaves, bandwidth);
  VW::example ec;
  auto ret_val = tree.predict(*as_singleline(test_base), ec);
  BOOST_CHECK_EQUAL(ret_val, expected_action);
  delete test_base;
}

bool operator!=(const VW::simple_label& lhs, const VW::simple_label& rhs) { return !(lhs.label == rhs.label); }

bool operator!=(const VW::simple_label_reduction_features& lhs, const VW::simple_label_reduction_features& rhs)
{
  return !(lhs.weight == rhs.weight && lhs.initial == rhs.initial);
}

BOOST_AUTO_TEST_CASE(otc_algo_learn_1_action_till_root)
{
  reduction_test_harness* pharness = nullptr;
  predictions_t preds_to_return = {1.f, -1.f};
  auto* base = get_test_harness_reduction(preds_to_return, pharness);
  VW::reductions::cats::cats_tree tree;
  tree.init(4, 0);

  VW::example ec;
  ec.ft_offset = 0;
  ec._debug_current_reduction_depth = 0;
  ec.l.cb = CB::label();
  ec.l.cb.costs.push_back(CB::cb_class{3.5f, 2, 0.5f});

  tree.learn(*as_singleline(base), ec);

  // verify 1) # of calls to learn 2) passed in labels 3) passed in weights
  vector<VW::simple_label> expected_labels = {{-1}, {1}};
  vector<float> expected_weights = {3.5f / 0.5f, 3.5f / 0.5f};

  BOOST_CHECK_EQUAL_COLLECTIONS(
      pharness->_labels.begin(), pharness->_labels.end(), expected_labels.begin(), expected_labels.end());

  BOOST_CHECK_EQUAL_COLLECTIONS(
      pharness->_weights.begin(), pharness->_weights.end(), expected_weights.begin(), expected_weights.end());

  // verify id of learners that were trained
  vector<uint64_t> expected_learners = {1, 0};
  BOOST_CHECK_EQUAL_COLLECTIONS(pharness->_learner_offset.begin(), pharness->_learner_offset.end(),
      expected_learners.begin(), expected_learners.end());

  delete base;
}

BOOST_AUTO_TEST_CASE(otc_algo_learn_1_action)
{
  reduction_test_harness* pharness = nullptr;
  predictions_t preds_to_return = {-1.f};
  auto* base = get_test_harness_reduction(preds_to_return, pharness);
  VW::reductions::cats::cats_tree tree;
  tree.init(4, 0);

  VW::example ec;
  ec.ft_offset = 0;
  ec._debug_current_reduction_depth = 0;
  ec.l.cb = CB::label();
  ec.l.cb.costs.push_back(CB::cb_class{3.5f, 2, 0.5f});

  tree.learn(*as_singleline(base), ec);

  // verify 1) # of calls to learn 2) passed in labels 3) passed in weights
  vector<VW::simple_label> expected_labels = {{-1}};
  vector<float> expected_weights = {3.5f / 0.5f};
  BOOST_CHECK_EQUAL_COLLECTIONS(
      pharness->_labels.begin(), pharness->_labels.end(), expected_labels.begin(), expected_labels.end());

  BOOST_CHECK_EQUAL_COLLECTIONS(
      pharness->_weights.begin(), pharness->_weights.end(), expected_weights.begin(), expected_weights.end());

  // verify id of learners that were trained
  vector<uint64_t> expected_learners = {1};
  BOOST_CHECK_EQUAL_COLLECTIONS(pharness->_learner_offset.begin(), pharness->_learner_offset.end(),
      expected_learners.begin(), expected_learners.end());

  delete base;
}

BOOST_AUTO_TEST_CASE(otc_algo_learn_2_action_siblings)
{
  VW::example ec;
  ec.ft_offset = 0;
  ec._debug_current_reduction_depth = 0;
  ec.l.cb = CB::label();
  ec.l.cb.costs.push_back(CB::cb_class{3.5f, 3, 0.5f});
  ec.l.cb.costs.push_back(CB::cb_class{3.5f, 4, 0.5f});

  predictions_t preds_to_return = {1.f, -1.f};

  reduction_test_harness* pharness = nullptr;
  auto* base = get_test_harness_reduction(preds_to_return, pharness);
  VW::reductions::cats::cats_tree tree;
  tree.init(8, 0);

  tree.learn(*as_singleline(base), ec);

  // verify 1) # of calls to learn 2) passed in labels 3) passed in weights
  vector<VW::simple_label> expected_labels = {{-1}, {1}};
  vector<float> expected_weights = {3.5f / 0.5f, 3.5f / 0.5f};

  BOOST_CHECK_EQUAL_COLLECTIONS(
      pharness->_labels.begin(), pharness->_labels.end(), expected_labels.begin(), expected_labels.end());
  BOOST_CHECK_EQUAL_COLLECTIONS(
      pharness->_weights.begin(), pharness->_weights.end(), expected_weights.begin(), expected_weights.end());

  // verify id of learners that were trained
  vector<uint64_t> expected_learners = {1, 0};
  BOOST_CHECK_EQUAL_COLLECTIONS(pharness->_learner_offset.begin(), pharness->_learner_offset.end(),
      expected_learners.begin(), expected_learners.end());

  delete base;
}

BOOST_AUTO_TEST_CASE(otc_algo_learn_2_action_notSiblings)
{
  VW::example ec;
  ec.ft_offset = 0;
  ec._debug_current_reduction_depth = 0;
  ec.l.cb = CB::label();
  ec.l.cb.costs.push_back(CB::cb_class{3.5f, 2, 0.5f});
  ec.l.cb.costs.push_back(CB::cb_class{3.5f, 3, 0.5f});

  predictions_t preds_to_return = {1.f, 1.f, -1.f, 1.f};

  reduction_test_harness* pharness = nullptr;
  auto* base = get_test_harness_reduction(preds_to_return, pharness);
  VW::reductions::cats::cats_tree tree;
  tree.init(8, 0);

  tree.learn(*as_singleline(base), ec);

  // verify 1) # of calls to learn 2) passed in labels 3) passed in weights
  vector<VW::simple_label> expected_labels = {{-1}, {1}, {1}, {1}};
  vector<float> expected_weights = {3.5f / 0.5f, 3.5f / 0.5f, 3.5f / 0.5f, 3.5f / 0.5f};

  BOOST_CHECK_EQUAL_COLLECTIONS(
      pharness->_labels.begin(), pharness->_labels.end(), expected_labels.begin(), expected_labels.end());
  BOOST_CHECK_EQUAL_COLLECTIONS(
      pharness->_weights.begin(), pharness->_weights.end(), expected_weights.begin(), expected_weights.end());

  // verify id of learners that were trained
  vector<uint64_t> expected_learners = {3, 4, 1, 0};
  BOOST_CHECK_EQUAL_COLLECTIONS(pharness->_learner_offset.begin(), pharness->_learner_offset.end(),
      expected_learners.begin(), expected_learners.end());

  delete base;
}

BOOST_AUTO_TEST_CASE(otc_algo_learn_2_action_notSiblings_bandwidth_1)
{
  VW::example ec;
  ec.ft_offset = 0;
  ec._debug_current_reduction_depth = 0;
  ec.l.cb = CB::label();
  ec.l.cb.costs.push_back(CB::cb_class{3.5f, 2, 0.5f});
  ec.l.cb.costs.push_back(CB::cb_class{3.5f, 3, 0.5f});

  predictions_t preds_to_return = {1.f, -1.f, 1.f};

  reduction_test_harness* pharness = nullptr;
  auto* base = get_test_harness_reduction(preds_to_return, pharness);
  VW::reductions::cats::cats_tree tree;
  tree.init(8, 1);

  tree.learn(*as_singleline(base), ec);

  // verify 1) # of calls to learn 2) passed in labels 3) passed in weights
  vector<VW::simple_label> expected_labels = {
      {1},
      {1},
      {1},
  };
  vector<float> expected_weights = {3.5f / 0.5f, 3.5f / 0.5f, 3.5f / 0.5f};

  BOOST_CHECK_EQUAL_COLLECTIONS(
      pharness->_labels.begin(), pharness->_labels.end(), expected_labels.begin(), expected_labels.end());
  BOOST_CHECK_EQUAL_COLLECTIONS(
      pharness->_weights.begin(), pharness->_weights.end(), expected_weights.begin(), expected_weights.end());

  // verify id of learners that were trained
  vector<uint64_t> expected_learners = {4, 1, 0};
  BOOST_CHECK_EQUAL_COLLECTIONS(pharness->_learner_offset.begin(), pharness->_learner_offset.end(),
      expected_learners.begin(), expected_learners.end());

  delete base;
}

BOOST_AUTO_TEST_CASE(otc_algo_learn_2_action_separate)
{
  VW::example ec;
  ec.ft_offset = 0;
  ec._debug_current_reduction_depth = 0;
  ec.l.cb = CB::label();
  ec.l.cb.costs.push_back(CB::cb_class{3.5f, 3, 0.5f});
  ec.l.cb.costs.push_back(CB::cb_class{3.5f, 6, 0.5f});

  predictions_t preds_to_return = {-1.f, -1.f, -1.f};

  reduction_test_harness* pharness = nullptr;
  auto* base = get_test_harness_reduction(preds_to_return, pharness);
  VW::reductions::cats::cats_tree tree;
  tree.init(8, 0);

  tree.learn(*as_singleline(base), ec);

  // verify 1) # of calls to learn 2) passed in labels 3) passed in weights
  vector<VW::simple_label> expected_labels = {{-1}, {1}, {-1}};
  vector<float> expected_weights = {3.5f / 0.5f, 3.5f / 0.5f, 3.5f / 0.5f};

  BOOST_CHECK_EQUAL_COLLECTIONS(
      pharness->_labels.begin(), pharness->_labels.end(), expected_labels.begin(), expected_labels.end());
  BOOST_CHECK_EQUAL_COLLECTIONS(
      pharness->_weights.begin(), pharness->_weights.end(), expected_weights.begin(), expected_weights.end());

  // verify id of learners that were trained
  vector<uint64_t> expected_learners = {1, 2, 0};
  BOOST_CHECK_EQUAL_COLLECTIONS(pharness->_learner_offset.begin(), pharness->_learner_offset.end(),
      expected_learners.begin(), expected_learners.end());

  delete base;
}

BOOST_AUTO_TEST_CASE(otc_algo_learn_2_action_separate_2)
{
  VW::example ec;
  ec.ft_offset = 0;
  ec._debug_current_reduction_depth = 0;
  ec.l.cb = CB::label();
  ec.l.cb.costs.push_back(CB::cb_class{3.5f, 3, 0.5f});
  ec.l.cb.costs.push_back(CB::cb_class{3.5f, 7, 0.5f});

  predictions_t preds_to_return = {1.f, 1.f, 1.f, -1.f};

  reduction_test_harness* pharness = nullptr;
  auto* base = get_test_harness_reduction(preds_to_return, pharness);
  VW::reductions::cats::cats_tree tree;
  tree.init(8, 0);

  tree.learn(*as_singleline(base), ec);

  // verify 1) # of calls to learn 2) passed in labels 3) passed in weights
  vector<VW::simple_label> expected_labels = {{1}, {-1}, {1}, {1}};
  vector<float> expected_weights = {3.5f / 0.5f, 3.5f / 0.5f, 3.5f / 0.5f, 3.5f / 0.5f};

  BOOST_CHECK_EQUAL_COLLECTIONS(
      pharness->_labels.begin(), pharness->_labels.end(), expected_labels.begin(), expected_labels.end());
  BOOST_CHECK_EQUAL_COLLECTIONS(
      pharness->_weights.begin(), pharness->_weights.end(), expected_weights.begin(), expected_weights.end());

  // verify id of learners that were trained
  vector<uint64_t> expected_learners = {6, 1, 2, 0};
  BOOST_CHECK_EQUAL_COLLECTIONS(pharness->_learner_offset.begin(), pharness->_learner_offset.end(),
      expected_learners.begin(), expected_learners.end());

  delete base;
}

BOOST_AUTO_TEST_CASE(otc_algo_learn_2_action_separate_bandwidth_2)
{
  VW::example ec;
  ec.ft_offset = 0;
  ec._debug_current_reduction_depth = 0;
  ec.l.cb = CB::label();
  ec.l.cb.costs.push_back(CB::cb_class{3.5f, 3, 0.5f});
  ec.l.cb.costs.push_back(CB::cb_class{3.5f, 6, 0.5f});

  predictions_t preds_to_return = {};

  reduction_test_harness* pharness = nullptr;
  auto* base = get_test_harness_reduction(preds_to_return, pharness);
  VW::reductions::cats::cats_tree tree;
  tree.init(8, 2);

  tree.learn(*as_singleline(base), ec);

  // verify 1) # of calls to learn 2) passed in labels 3) passed in weights
  vector<VW::simple_label> expected_labels = {};
  vector<float> expected_weights = {};

  BOOST_CHECK_EQUAL_COLLECTIONS(
      pharness->_labels.begin(), pharness->_labels.end(), expected_labels.begin(), expected_labels.end());
  BOOST_CHECK_EQUAL_COLLECTIONS(
      pharness->_weights.begin(), pharness->_weights.end(), expected_weights.begin(), expected_weights.end());

  // verify id of learners that were trained
  vector<uint64_t> expected_learners = {};
  BOOST_CHECK_EQUAL_COLLECTIONS(pharness->_learner_offset.begin(), pharness->_learner_offset.end(),
      expected_learners.begin(), expected_learners.end());

  delete base;
}

BOOST_AUTO_TEST_CASE(otc_algo_learn_2_action_separate_2_bandwidth_2)
{
  VW::example ec;
  ec.ft_offset = 0;
  ec._debug_current_reduction_depth = 0;
  ec.l.cb = CB::label();
  ec.l.cb.costs.push_back(CB::cb_class{3.5f, 3, 0.5f});
  ec.l.cb.costs.push_back(CB::cb_class{3.5f, 11, 0.5f});

  predictions_t preds_to_return = {1, 1, -1};

  reduction_test_harness* pharness = nullptr;
  auto* base = get_test_harness_reduction(preds_to_return, pharness);
  VW::reductions::cats::cats_tree tree;
  tree.init(16, 2);

  tree.learn(*as_singleline(base), ec);

  // verify 1) # of calls to learn 2) passed in labels 3) passed in weights
  vector<VW::simple_label> expected_labels = {{1}, {1}, {1}};
  vector<float> expected_weights = {3.5f / 0.5f, 3.5f / 0.5f, 3.5f / 0.5f};

  BOOST_CHECK_EQUAL_COLLECTIONS(
      pharness->_labels.begin(), pharness->_labels.end(), expected_labels.begin(), expected_labels.end());
  BOOST_CHECK_EQUAL_COLLECTIONS(
      pharness->_weights.begin(), pharness->_weights.end(), expected_weights.begin(), expected_weights.end());

  // verify id of learners that were trained
  vector<uint64_t> expected_learners = {12, 5, 0};
  BOOST_CHECK_EQUAL_COLLECTIONS(pharness->_learner_offset.begin(), pharness->_learner_offset.end(),
      expected_learners.begin(), expected_learners.end());

  delete base;
}

BOOST_AUTO_TEST_CASE(otc_algo_learn_2_action_separate_bandwidth_1_asym)
{
  VW::example ec;
  ec.ft_offset = 0;
  ec._debug_current_reduction_depth = 0;
  ec.l.cb = CB::label();
  ec.l.cb.costs.push_back(CB::cb_class{3.5f, 2, 0.5f});
  ec.l.cb.costs.push_back(CB::cb_class{3.5f, 5, 0.5f});

  predictions_t preds_to_return = {-1.f, 1.f, -1.f};

  reduction_test_harness* pharness = nullptr;
  auto* base = get_test_harness_reduction(preds_to_return, pharness);
  VW::reductions::cats::cats_tree tree;
  tree.init(8, 1);

  tree.learn(*as_singleline(base), ec);

  // verify 1) # of calls to learn 2) passed in labels 3) passed in weights
  vector<VW::simple_label> expected_labels = {
      {1},
      {1},
      {1},
  };
  vector<float> expected_weights = {3.5f / 0.5f, 3.5f / 0.5f, 3.5f / 0.5f};

  BOOST_CHECK_EQUAL_COLLECTIONS(
      pharness->_labels.begin(), pharness->_labels.end(), expected_labels.begin(), expected_labels.end());
  BOOST_CHECK_EQUAL_COLLECTIONS(
      pharness->_weights.begin(), pharness->_weights.end(), expected_weights.begin(), expected_weights.end());

  // verify id of learners that were trained
  vector<uint64_t> expected_learners = {5, 2, 0};
  BOOST_CHECK_EQUAL_COLLECTIONS(pharness->_learner_offset.begin(), pharness->_learner_offset.end(),
      expected_learners.begin(), expected_learners.end());

  delete base;
}

BOOST_AUTO_TEST_CASE(offset_tree_cont_predict)
{
  // 0 node tree
  predict_test_helper({}, 0, 0, 0);
  // 2 node trees
  predict_test_helper({-1}, 1, 2, 0);
  predict_test_helper({1}, 2, 2, 0);
  // 4 node tree
  predict_test_helper({-1, 1}, 2, 4, 0);
  predict_test_helper({1, 1}, 4, 4, 0);
  // 4 node tree with bandwidth 1
  predict_test_helper({-1}, 2, 4, 1);
  predict_test_helper({1}, 3, 4, 1);
  // 8 node tree with bandwidth 1
  predict_test_helper({-1, -1}, 2, 8, 1);
  predict_test_helper({-1, 1, -1}, 3, 8, 1);
  // 8 node tree with bandwidth 2
  predict_test_helper({-1, -1}, 3, 8, 2);
  predict_test_helper({1, 1}, 6, 8, 2);
}

BOOST_AUTO_TEST_CASE(build_min_depth_tree_cont_5)
{
  VW::reductions::cats::min_depth_binary_tree tree;
  tree.build_tree(4, 1);
  std::vector<VW::reductions::cats::tree_node> expected = {
      {0, 1, 2, 0, 0, false, false, false},
      {1, 3, 4, 0, 1, false, true, false},
      {2, 5, 6, 0, 1, true, false, false},
      {3, 0, 0, 1, 2, false, false, true},
      {4, 0, 0, 1, 2, false, false, true},
      {5, 0, 0, 2, 2, false, false, true},
      {6, 0, 0, 2, 2, false, false, true},
  };
  BOOST_CHECK_EQUAL_COLLECTIONS(tree.nodes.begin(), tree.nodes.end(), expected.begin(), expected.end());
}

BOOST_AUTO_TEST_CASE(build_min_depth_tree_cont_1)
{
  VW::reductions::cats::min_depth_binary_tree tree;
  tree.build_tree(1, 0);
  std::vector<VW::reductions::cats::tree_node> expected = {{0, 0, 0, 0, 0, false, false, true}};
  BOOST_CHECK_EQUAL_COLLECTIONS(tree.nodes.begin(), tree.nodes.end(), expected.begin(), expected.end());
}
