#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
#   define BOOST_TEST_MODULE Main
#endif

#include "ranking_response.h"
#include <boost/test/unit_test.hpp>
#include "api_status.h"

using namespace reinforcement_learning;
using namespace std;

using datacol = vector<pair<int, float>>;

datacol get_test_data() {
  return {
    { 2,1.1f },
    { 6,0.1f },
    { 1,0.9f },
    { 4,2.1f },
    { 3,3.1f } 
  };
}

BOOST_AUTO_TEST_CASE(ranking_response_uuid) {
  ranking_response resp;
  BOOST_CHECK_EQUAL(resp.get_uuid(), "");
  ranking_response resp2("uuid");
  BOOST_CHECK_EQUAL(resp2.get_uuid(), "uuid");
}

BOOST_AUTO_TEST_CASE(ranking_response_empty_collection) {
  ranking_response resp;
  api_status s;
  auto action_id = resp.get_top_action_id(&s);
  BOOST_CHECK(s.get_error_code() > 0);
  BOOST_CHECK_EQUAL(action_id, -1);

  action_id = 10;
  action_id = resp.get_top_action_id();
  BOOST_CHECK_EQUAL(action_id, -1);
}

BOOST_AUTO_TEST_CASE(ranking_response_write_read_iterator) {
  auto test_data = get_test_data();

  ranking_response resp;

  for( auto& p : test_data ) {
    resp.push_back(p.first, p.second);
  }

  int i = 0;
  for( auto&& d: resp) {
    auto& td = test_data[i++];
    BOOST_CHECK_EQUAL(d.action_id,td.first);
    BOOST_CHECK_EQUAL(d.probability, td.second);
  }

  i = 0;
  for(auto r = begin(resp); r != end(resp); ++r ) {
    auto& td = test_data[i++];
    BOOST_CHECK_EQUAL(( *r ).action_id, td.first);
    BOOST_CHECK_EQUAL(( *r ).probability, td.second);
  }
}

