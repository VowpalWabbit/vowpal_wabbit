#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
#   define BOOST_TEST_MODULE Main
#endif

#include "logger/moving_queue.h"
#include <boost/test/unit_test.hpp>

using namespace reinforcement_learning;
using namespace std;

class test_msg : public event {
public:
  test_msg() {}
  test_msg(const char* id) : event(id) {}

  bool try_drop(float drop_prob, int _drop_pass) override {
    return _event_id.substr(0, 4) == "drop";
  }

  std::string str() override {
    return _event_id;
  }
};

BOOST_AUTO_TEST_CASE(push_pop_test) {
  moving_queue<test_msg> queue;
  queue.push(test_msg("1"));
  queue.push(test_msg("2"));
  queue.push(test_msg("3"));

  test_msg val;

  BOOST_CHECK_EQUAL(queue.size(), 3);
  queue.pop(&val);
  BOOST_CHECK_EQUAL(val.str(), "1");

  BOOST_CHECK_EQUAL(queue.size(), 2);
  queue.pop(&val);
  BOOST_CHECK_EQUAL(val.str(), "2");

  BOOST_CHECK_EQUAL(queue.size(), 1);
  queue.pop(&val);
  BOOST_CHECK_EQUAL(val.str(), "3");
  BOOST_CHECK_EQUAL(queue.size(), 0);
}

BOOST_AUTO_TEST_CASE(prune_test) {
  moving_queue<test_msg> queue;
  queue.push(test_msg("no_drop_1"));
  queue.push(test_msg("drop_1"));
  queue.push(test_msg("no_drop_2"));
  queue.push(test_msg("drop_2"));
  queue.push(test_msg("no_drop_3"));

  test_msg val;

  BOOST_CHECK_EQUAL(queue.size(), 5);
  queue.prune(1.0);

  queue.pop(&val);
  BOOST_CHECK_EQUAL(val.str(), "no_drop_1");

  queue.pop(&val);
  BOOST_CHECK_EQUAL(val.str(), "no_drop_2");

  queue.pop(&val);
  BOOST_CHECK_EQUAL(val.str(), "no_drop_3");
}
