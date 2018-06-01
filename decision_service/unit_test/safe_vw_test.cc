#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
#   define BOOST_TEST_MODULE Main
#endif

#include <boost/test/unit_test.hpp>
// #include <boost/test/test_tools.hpp>
#include "safe_vw.h"
#include "object_pool.h"
#include "data.h"

#include <iostream>

using namespace reinforcement_learning;

BOOST_AUTO_TEST_CASE(safe_vw_1)
{
  safe_vw vw((const char*)cb_data_5_model, cb_data_5_model_len);
  const char* json = R"({"a":{"0":1,"5":2},"_multi":[{"b":{"0":1}},{"b":{"0":2}},{"b":{"0":3}}]})";

  std::vector<float> ranking = vw.rank(json);
  std::vector<float> ranking_expected = { .1f, .1f, .8f };

  // doesn't work with Boost 1.58
  // BOOST_TEST(ranking == ranking_expected, boost::test_tools::per_element());
  BOOST_CHECK_EQUAL_COLLECTIONS(ranking.begin(), ranking.end(),
    ranking_expected.begin(), ranking_expected.end());
}

BOOST_AUTO_TEST_CASE(safe_vw_factory_1)
{
  const char* json = R"({"a":{"0":1,"5":2},"_multi":[{"b":{"0":1}},{"b":{"0":2}},{"b":{"0":3}}]})";
  std::vector<float> ranking_expected = { .1f, .1f, .8f };

  safe_vw_factory* factory = new safe_vw_factory(std::make_shared<safe_vw>((const char*)cb_data_5_model, cb_data_5_model_len));
  object_pool<safe_vw, safe_vw_factory> pool(factory);

  {
    pooled_object_guard<safe_vw, safe_vw_factory> guard(pool, pool.get_or_create());

    // update factory while an object is floating around
    pool.update_factory(new safe_vw_factory(std::make_shared<safe_vw>((const char*)cb_data_5_model, cb_data_5_model_len)));

    std::vector<float> ranking = guard->rank(json);
    BOOST_CHECK_EQUAL_COLLECTIONS(ranking.begin(), ranking.end(), ranking_expected.begin(), ranking_expected.end());
  }

  {
    // make sure we get a new object
    pooled_object_guard<safe_vw, safe_vw_factory> guard(pool, pool.get_or_create());

    std::vector<float> ranking = guard->rank(json);
    BOOST_CHECK_EQUAL_COLLECTIONS(ranking.begin(), ranking.end(), ranking_expected.begin(), ranking_expected.end());
  }
}
