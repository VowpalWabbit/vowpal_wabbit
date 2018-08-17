#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
#   define BOOST_TEST_MODULE Main
#endif

#include <boost/test/unit_test.hpp>
#include "vw_model/safe_vw.h"
#include "utility/object_pool.h"
#include "data.h"

using namespace reinforcement_learning;
using namespace reinforcement_learning::utility;

using vw_ptr = std::shared_ptr<safe_vw>;
using pooled_vw = pooled_object_guard<safe_vw, safe_vw_factory>;

BOOST_AUTO_TEST_CASE(safe_vw_1)
{
  safe_vw vw((const char*)cb_data_5_model, cb_data_5_model_len);
  const auto json = R"({"a":{"0":1,"5":2},"_multi":[{"b":{"0":1}},{"b":{"0":2}},{"b":{"0":3}}]})";

  std::vector<int> actions;
  std::vector<float> ranking;
  vw.rank(json, actions, ranking);

  std::vector<float> ranking_expected = { .8f, .1f, .1f };

  BOOST_CHECK_EQUAL_COLLECTIONS(ranking.begin(), ranking.end(),
    ranking_expected.begin(), ranking_expected.end());
}

BOOST_AUTO_TEST_CASE(factory_with_initial_model)
{
  const auto json = R"({"a":{"0":1,"5":2},"_multi":[{"b":{"0":1}},{"b":{"0":2}},{"b":{"0":3}}]})";
  std::vector<float> ranking_expected = { .8f, .1f, .1f };

  // Start with an initial model
  const auto initial_model = std::make_shared<safe_vw>((const char*)cb_data_5_model, cb_data_5_model_len);
  const auto factory = new safe_vw_factory(initial_model);
  object_pool<safe_vw, safe_vw_factory> pool(factory);

  {
    pooled_vw guard(pool, pool.get_or_create());

    // Update factory while an object is floating around
    const auto updated_model = std::make_shared<safe_vw>((const char*)cb_data_5_model, cb_data_5_model_len);
    pool.update_factory(new safe_vw_factory(updated_model));

    std::vector<int> actions;
    std::vector<float> ranking;
    guard->rank(json, actions, ranking);

    BOOST_CHECK_EQUAL_COLLECTIONS(ranking.begin(), ranking.end(), ranking_expected.begin(), ranking_expected.end());
  }

  {
    // Make sure we get a new object
    pooled_object_guard<safe_vw, safe_vw_factory> guard(pool, pool.get_or_create());

    std::vector<int> actions;
    std::vector<float> ranking;
    guard->rank(json, actions, ranking);

    BOOST_CHECK_EQUAL_COLLECTIONS(ranking.begin(), ranking.end(), ranking_expected.begin(), ranking_expected.end());
  }
}

BOOST_AUTO_TEST_CASE(factory_with_empty_model) {
  const auto json = R"({"a":{"0":1,"5":2},"_multi":[{"b":{"0":1}},{"b":{"0":2}},{"b":{"0":3}}]})";
  std::vector<float> ranking_expected = { .8f, .1f, .1f };

  // Start with empty model
  const vw_ptr empty(nullptr);
  const auto factory = new safe_vw_factory(empty);
  object_pool<safe_vw, safe_vw_factory> pool(factory);

  // Initial model & rank call
  {
    const auto new_model = std::make_shared<safe_vw>((const char*)cb_data_5_model, cb_data_5_model_len);
    pool.update_factory(new safe_vw_factory(new_model));
    pooled_vw vw(pool, pool.get_or_create());

    std::vector<int> actions;
    std::vector<float> ranking;
    vw->rank(json, actions, ranking);

    BOOST_CHECK_EQUAL_COLLECTIONS(ranking.begin(), ranking.end(), ranking_expected.begin(), ranking_expected.end());
  }

  // Update model & rank call
  {
    const auto new_model = std::make_shared<safe_vw>((const char*)cb_data_5_model, cb_data_5_model_len);
    pool.update_factory(new safe_vw_factory(new_model));
    pooled_vw vw(pool, pool.get_or_create());

    std::vector<int> actions;
    std::vector<float> ranking;
    vw->rank(json, actions, ranking);

    BOOST_CHECK_EQUAL_COLLECTIONS(ranking.begin(), ranking.end(), ranking_expected.begin(), ranking_expected.end());
  }
}
