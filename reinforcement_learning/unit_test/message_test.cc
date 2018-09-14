#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
#   define BOOST_TEST_MODULE Main
#endif

#include "ranking_event.h"
#include "utility/data_buffer.h"
#include "ranking_event.h"
#include "ranking_response.h"
#include <boost/test/unit_test.hpp>
#include <string>

using namespace reinforcement_learning;
using namespace reinforcement_learning::utility;
using namespace std;

BOOST_AUTO_TEST_CASE(interaction_message_survive_test) {
  data_buffer buffer;
  data_buffer expected_buffer;
  ranking_response resp("interaction_id");
  resp.push_back(1, 0.1);
  resp.push_back(2, 0.2);
  resp.set_chosen_action_id(1);

  ranking_event evt(buffer, "interaction_id", "interaction_context", resp);

  ranking_event expected(expected_buffer, "interaction_id", "interaction_context", resp, 0.75);

  evt.try_drop(0.5, 1);
  evt.try_drop(0.5, 1);

  BOOST_CHECK_EQUAL(evt.str(), expected.str());
}

