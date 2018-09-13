#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
#   define BOOST_TEST_MODULE Main
#endif

#include "logger/moving_queue.h"
#include <boost/test/unit_test.hpp>

using namespace reinforcement_learning;
using namespace std;

class test_msg {
public:
  int _value{ 0 };

  test_msg() {}
  test_msg(int value) : _value(value) {}

  bool try_drop(float drop_prob, int _drop_pass) {
    return _value % 2 == 0;
  }
};

BOOST_AUTO_TEST_CASE(push_pop_test) {
  moving_queue<test_msg> queue;
  queue.push(test_msg(1));
  queue.push(test_msg(2));
  queue.push(test_msg(3));

  test_msg val;

  BOOST_CHECK_EQUAL(queue.size(), 3);
  queue.pop(&val);
  BOOST_CHECK_EQUAL(val._value, 1);

  BOOST_CHECK_EQUAL(queue.size(), 2);
  queue.pop(&val);
  BOOST_CHECK_EQUAL(val._value, 2);

  BOOST_CHECK_EQUAL(queue.size(), 1);
  queue.pop(&val);
  BOOST_CHECK_EQUAL(val._value, 3);
  BOOST_CHECK_EQUAL(queue.size(), 0);
}

BOOST_AUTO_TEST_CASE(prune_test) {
  moving_queue<test_msg> queue;
  queue.push(test_msg(1));
  queue.push(test_msg(2));
  queue.push(test_msg(3));
  queue.push(test_msg(4));
  queue.push(test_msg(5));

  test_msg val;

  BOOST_CHECK_EQUAL(queue.size(), 5);
  queue.prune(1.0);

  queue.pop(&val);
  BOOST_CHECK_EQUAL(val._value, 1);

  queue.pop(&val);
  BOOST_CHECK_EQUAL(val._value, 3);

  queue.pop(&val);
  BOOST_CHECK_EQUAL(val._value, 5);
}
