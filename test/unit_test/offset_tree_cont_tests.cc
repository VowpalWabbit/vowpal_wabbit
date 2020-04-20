#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>
#include "offset_tree_cont_prev.h"
#include "offset_tree_cont_tests.h"

using namespace LEARNER;
using namespace std;

namespace VW
{
namespace offset_tree_cont
{
std::ostream& operator<<(std::ostream& os, const tree_node& node)
{
  os << "{" << node.id << "," << node.left_id << "," << node.right_id << ", " << node.parent_id << ", " << node.depth
     << ", " << (node.is_leaf ? "true" : "false") << "}";
  return os;
}

struct reduction_test_harness
{
  reduction_test_harness() : _curr_idx(0) {}

  void set_predict_response(const vector<float>& predictions) { _predictions = predictions; }

  void test_predict(single_learner& base, example& ec) { ec.pred.scalar = _predictions[_curr_idx++]; }

  void test_learn(single_learner& base, example& ec)
  {
    _labels.emplace_back(ec.l.simple);
    _labels.back().weight = ec.weight;
    _learner_offset.emplace_back(ec.ft_offset);
  }

  static void predict(reduction_test_harness& test_reduction, single_learner& base, example& ec)
  {
    test_reduction.test_predict(base, ec);
  }

  static void learn(reduction_test_harness& test_reduction, single_learner& base, example& ec)
  {
    test_reduction.test_learn(base, ec);
  };

  vector<float> _predictions;
  vector<label_data> _labels;
  vector<uint64_t> _learner_offset;
  int _curr_idx;
};

using test_learner_t = learner<reduction_test_harness, example>;
using predictions_t = vector<float>;
using scores_t = int;

void predict_test_helper(
    const predictions_t& base_reduction_predictions, const scores_t& expected_action, uint32_t num_leaves);

template <typename T = reduction_test_harness>
learner<T, example>* get_test_harness_reduction(const predictions_t& base_reduction_predictions)
{
  T* pharness = nullptr;
  return get_test_harness_reduction(base_reduction_predictions, pharness);
}

    template <typename T = reduction_test_harness>
learner<T, example>* get_test_harness_reduction(const predictions_t& base_reduction_predictions, T*& pharness)
{
  // Setup a test harness base reduction
  auto test_harness = scoped_calloc_or_throw<T>();
  pharness = test_harness.get();
  test_harness->set_predict_response(base_reduction_predictions);
  auto& test_learner =
      init_learner(test_harness,  // Data structure passed by vw_framework into test_harness predict/learn calls
          T::learn,               // test_harness learn
          T::predict,             // test_harness predict
          1                       // Number of regressors in test_harness (not used)
      );                          // Create a learner using the base reduction.
  return &test_learner;
}

}  // namespace offset_tree_cont
}  // namespace VW

using namespace VW::offset_tree_cont;

bool operator!=(const label_data& lhs, const label_data& rhs)
{
  return !(lhs.label == rhs.label && lhs.weight == rhs.weight && lhs.initial == rhs.initial);
}

std::ostream& operator<<(std::ostream& o, label_data const& lbl)
{
  o << "{l=" << lbl.label << ", w=" << lbl.weight << ", i=" << lbl.initial << "}";
  return o;
}

BOOST_AUTO_TEST_CASE(otc_algo_learn_1_action_till_root)
{
  reduction_test_harness* pharness = nullptr;
  predictions_t preds_to_return = {1.f,-1.f};
  auto& base = *as_singleline(get_test_harness_reduction(preds_to_return, pharness));
  offset_tree tree;
  tree.init(5);

  example ec;
  ec.ft_offset = 0;
  ec.stack_depth = 0;
  ec.l.cb = CB::label();
  ec.l.cb.costs.push_back({3.5f, 1, 0.5f, 0.0f});

  tree.learn(base, ec);

  // verify 1) # of calls to learn 2) passed in labels 3) passed in weights
  vector<label_data> expected_labels = {
    {-1, 3.5f / .5f, 0},
    {1, 3.5f / .5f, 0}
  };

  BOOST_CHECK_EQUAL_COLLECTIONS(
      pharness->_labels.begin(), pharness->_labels.end(), expected_labels.begin(), expected_labels.end());

  // verify id of learners that were trained
  vector<uint64_t> expected_learners = {1, 0};
  BOOST_CHECK_EQUAL_COLLECTIONS(pharness->_learner_offset.begin(), pharness->_learner_offset.end(),
      expected_learners.begin(), expected_learners.end());

  destroy_free<test_learner_t>(&base);
}

BOOST_AUTO_TEST_CASE(otc_algo_learn_1_action)
{
  reduction_test_harness* pharness = nullptr;
  predictions_t preds_to_return = {-1.f};
  auto& base = *as_singleline(get_test_harness_reduction(preds_to_return,pharness));
  offset_tree tree;
  tree.init(5);

  example ec;
  ec.ft_offset = 0;
  ec.stack_depth = 0;
  ec.l.cb = CB::label();
  ec.l.cb.costs.push_back({3.5f, 1, 0.5f, 0.0f});

  tree.learn(base, ec);

  // verify 1) # of calls to learn 2) passed in labels 3) passed in weights
  vector<label_data> expected_labels = { {-1,3.5f/.5f,0} };
  BOOST_CHECK_EQUAL_COLLECTIONS(
    pharness->_labels.begin(),
    pharness->_labels.end(),
    expected_labels.begin(),
    expected_labels.end());

  // verify id of learners that were trained
  vector<uint64_t> expected_learners = {1};
  BOOST_CHECK_EQUAL_COLLECTIONS(
    pharness->_learner_offset.begin(),
    pharness->_learner_offset.end(),
    expected_learners.begin(),
    expected_learners.end());

  destroy_free<test_learner_t>(&base);
}

BOOST_AUTO_TEST_CASE(otc_algo_learn_2_action_split)
{
  example ec;
  ec.ft_offset = 0;
  ec.stack_depth = 0;
  ec.l.cb = CB::label();
  ec.l.cb.costs.push_back({3.5f, 3, 0.5f, 0.0f});
  ec.l.cb.costs.push_back({3.5f, 4, 0.5f, 0.0f});

  predictions_t preds_to_return = {1.f, -1.f};

  reduction_test_harness* pharness = nullptr;
  auto& base = *as_singleline(get_test_harness_reduction(preds_to_return, pharness));
  offset_tree tree;
  tree.init(5);

  tree.learn(base, ec);

  // verify 1) # of calls to learn 2) passed in labels 3) passed in weights
  vector<label_data> expected_labels = {{1, 3.5f / .5f, 0}, {-1, 3.5f / .5f, 0}};
  BOOST_CHECK_EQUAL_COLLECTIONS(
      pharness->_labels.begin(), pharness->_labels.end(), expected_labels.begin(), expected_labels.end());

  // verify id of learners that were trained
  vector<uint64_t> expected_learners = {3, 2};
  BOOST_CHECK_EQUAL_COLLECTIONS(pharness->_learner_offset.begin(), pharness->_learner_offset.end(),
      expected_learners.begin(), expected_learners.end());

  destroy_free<test_learner_t>(&base);
}

BOOST_AUTO_TEST_CASE(otc_algo_learn_2_action_split_till_root)
{
  example ec;
  ec.ft_offset = 0;
  ec.stack_depth = 0;
  ec.l.cb = CB::label();
  ec.l.cb.costs.push_back({3.5f, 3, 0.5f, 0.0f});
  ec.l.cb.costs.push_back({3.5f, 4, 0.5f, 0.0f});

  predictions_t preds_to_return = {-1.f, 1.f, -1.f};

  reduction_test_harness* pharness = nullptr;
  auto& base = *as_singleline(get_test_harness_reduction(preds_to_return, pharness));
  offset_tree tree;
  tree.init(5);

  tree.learn(base, ec);

  // verify 1) # of calls to learn 2) passed in labels 3) passed in weights
  vector<label_data> expected_labels = {
    {1, 3.5f / .5f, 0},
    {-1, 3.5f / .5f, 0},
    {1, 3.5f / .5f, 0}
  };
  BOOST_CHECK_EQUAL_COLLECTIONS(
      pharness->_labels.begin(), pharness->_labels.end(), expected_labels.begin(), expected_labels.end());

  // verify id of learners that were trained
  vector<uint64_t> expected_learners = {3, 2, 1};
  BOOST_CHECK_EQUAL_COLLECTIONS(pharness->_learner_offset.begin(), pharness->_learner_offset.end(),
      expected_learners.begin(), expected_learners.end());

  destroy_free<test_learner_t>(&base);
}

BOOST_AUTO_TEST_CASE(otc_algo_learn_2_action_split_till_root_b)
{
  example ec;
  ec.ft_offset = 0;
  ec.stack_depth = 0;
  ec.l.cb = CB::label();
  ec.l.cb.costs.push_back({3.5f, 3, 0.5f, 0.0f});
  ec.l.cb.costs.push_back({3.5f, 4, 0.5f, 0.0f});

  predictions_t preds_to_return = {-1.f, -1.f, -1.f, 1.f};

  reduction_test_harness* pharness = nullptr;
  auto& base = *as_singleline(get_test_harness_reduction(preds_to_return, pharness));
  offset_tree tree;
  tree.init(5);

  tree.learn(base, ec);

  // verify 1) # of calls to learn 2) passed in labels 3) passed in weights
  vector<label_data> expected_labels = {
    {1, 3.5f / .5f, 0},
    {-1, 3.5f / .5f, 0},
    {1, 3.5f / .5f, 0},
    {1, 3.5f / .5f, 0},
  };
  BOOST_CHECK_EQUAL_COLLECTIONS(
      pharness->_labels.begin(), pharness->_labels.end(), expected_labels.begin(), expected_labels.end());

  // verify id of learners that were trained
  vector<uint64_t> expected_learners = {3, 2, 1, 0};
  BOOST_CHECK_EQUAL_COLLECTIONS(pharness->_learner_offset.begin(), pharness->_learner_offset.end(),
      expected_learners.begin(), expected_learners.end());

  destroy_free<test_learner_t>(&base);
}

BOOST_AUTO_TEST_CASE(otc_algo_learn_2_action)
{
  example ec;
  ec.ft_offset = 0;
  ec.stack_depth = 0;
  ec.l.cb = CB::label();
  ec.l.cb.costs.push_back({3.5f, 1, 0.5f, 0.0f});
  ec.l.cb.costs.push_back({3.5f, 2, 0.5f, 0.0f});

  predictions_t preds_to_return = {1.f, -1.f};

  reduction_test_harness* pharness = nullptr;
  auto& base = *as_singleline(get_test_harness_reduction(preds_to_return, pharness));
  offset_tree tree;
  tree.init(5);

  tree.learn(base, ec);

  // verify 1) # of calls to learn 2) passed in labels 3) passed in weights
  vector<label_data> expected_labels = {
    {1, 3.5f / .5f, 0},
    {-1, 3.5f / .5f, 0}};
  BOOST_CHECK_EQUAL_COLLECTIONS(
      pharness->_labels.begin(), pharness->_labels.end(), expected_labels.begin(), expected_labels.end());

  // verify id of learners that were trained
  vector<uint64_t> expected_learners = {2,1};
  BOOST_CHECK_EQUAL_COLLECTIONS(pharness->_learner_offset.begin(), pharness->_learner_offset.end(),
      expected_learners.begin(), expected_learners.end());

  destroy_free<test_learner_t>(&base);
}

BOOST_AUTO_TEST_CASE(otc_algo_learn_3_action)
{
  example ec;
  ec.ft_offset = 0;
  ec.stack_depth = 0;
  ec.l.cb = CB::label();
  ec.l.cb.costs.push_back({3.5f, 1, 0.5f, 0.0f});
  ec.l.cb.costs.push_back({3.5f, 2, 0.5f, 0.0f});
  ec.l.cb.costs.push_back({3.5f, 3, 0.5f, 0.0f});

  predictions_t preds_to_return = {-1.f, 1.f};

  reduction_test_harness* pharness = nullptr;
  auto& base = *as_singleline(get_test_harness_reduction(preds_to_return, pharness));
  offset_tree tree;
  tree.init(5);

  tree.learn(base, ec);

  // verify 1) # of calls to learn 2) passed in labels 3) passed in weights
  vector<label_data> expected_labels = {
    {-1, 3.5f / .5f, 0},
    {-1, 3.5f / .5f, 0}
  };

  BOOST_CHECK_EQUAL_COLLECTIONS(
      pharness->_labels.begin(), pharness->_labels.end(), expected_labels.begin(), expected_labels.end());

  // verify id of learners that were trained
  vector<uint64_t> expected_learners = {1,0};
  BOOST_CHECK_EQUAL_COLLECTIONS(pharness->_learner_offset.begin(), pharness->_learner_offset.end(),
      expected_learners.begin(), expected_learners.end());

  destroy_free<test_learner_t>(&base);
}

BOOST_AUTO_TEST_CASE(otc_algo_learn_2_action_till_root)
{
  example ec;
  ec.ft_offset = 0;
  ec.stack_depth = 0;
  ec.l.cb = CB::label();
  ec.l.cb.costs.push_back({3.5f, 1, 0.5f, 0.0f});
  ec.l.cb.costs.push_back({3.5f, 2, 0.5f, 0.0f});

  predictions_t preds_to_return = {-1.f, 1.f};

  reduction_test_harness* pharness = nullptr;
  auto& base = *as_singleline(get_test_harness_reduction(preds_to_return, pharness));
  offset_tree tree;
  tree.init(5);

  tree.learn(base, ec);

  // verify 1) # of calls to learn 2) passed in labels 3) passed in weights
  vector<label_data> expected_labels = {
    {1, 3.5f / .5f, 0},
    {-1, 3.5f / .5f, 0}};
  BOOST_CHECK_EQUAL_COLLECTIONS(
      pharness->_labels.begin(), pharness->_labels.end(), expected_labels.begin(), expected_labels.end());

  // verify id of learners that were trained
  vector<uint64_t> expected_learners = {2, 1};
  BOOST_CHECK_EQUAL_COLLECTIONS(pharness->_learner_offset.begin(), pharness->_learner_offset.end(),
      expected_learners.begin(), expected_learners.end());

  destroy_free<test_learner_t>(&base);
}

BOOST_AUTO_TEST_CASE(offset_tree_cont_learn_basic_1)
{
  // Setup a test harness base reduction
  const auto test_harness = VW::offset_tree_cont::get_test_harness_reduction({-1, 1});

  offset_tree tree;
  tree.init(3);
  example ec;
  ec.l.cb = CB::label();
  ec.l.cb.costs.push_back({2.0f, 2, 0.5f, 0.0f});
  ec.l.cb.costs.push_back({3.5f, 1, 0.5f, 0.0f});

  tree.learn(*as_singleline(test_harness), ec);

  destroy_free<test_learner_t>(test_harness);
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

BOOST_AUTO_TEST_CASE(offset_tree_cont_predict)
{
  // 0 node tree
  VW::offset_tree_cont::predict_test_helper({}, 0, 0);
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

BOOST_AUTO_TEST_CASE(build_min_depth_tree_cont_5)
{
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

BOOST_AUTO_TEST_CASE(build_min_depth_tree_cont_4)
{
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

BOOST_AUTO_TEST_CASE(build_min_depth_tree_cont_3)
{
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

BOOST_AUTO_TEST_CASE(build_min_depth_tree_cont_1)
{
  VW::offset_tree_cont::min_depth_binary_tree tree;
  tree.build_tree(1);
  std::vector<VW::offset_tree_cont::tree_node> expected = {
      {0, 0, 0, 0, 0, true}
  };
  BOOST_CHECK_EQUAL_COLLECTIONS(tree.nodes.begin(), tree.nodes.end(), expected.begin(), expected.end());
}

BOOST_AUTO_TEST_CASE(build_min_depth_tree_cont_too_big)
{
  VW::offset_tree_cont::min_depth_binary_tree tree;
  // Throws vw_exception when unable to allocate enough memory to build tree
  BOOST_CHECK_THROW(tree.build_tree(INT_MAX), VW::vw_exception);
}

namespace VW
{
namespace offset_tree_cont
{
void predict_test_helper(
    const predictions_t& base_reduction_predictions, const scores_t& expected_action, uint32_t num_leaves)
{
  const auto test_base = get_test_harness_reduction(base_reduction_predictions);
  VW::offset_tree_cont::offset_tree tree;
  tree.init(num_leaves);
  example ec;
  ec.l.cb.costs = v_init<CB::cb_class>();
  auto ret_val = tree.predict(*as_singleline(test_base), ec);
  BOOST_CHECK_EQUAL(ret_val, expected_action);
  destroy_free<test_learner_t>(test_base);
}
}  // namespace offset_tree_cont
}  // namespace VW
