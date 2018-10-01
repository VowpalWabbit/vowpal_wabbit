#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
#   define BOOST_TEST_MODULE Main
#endif

#include "moving_queue.h"
#include <boost/test/unit_test.hpp>

using namespace reinforcement_learning;
using namespace std;

class move_only {
public:
  move_only(int value = 0) : _value(value) {}

  move_only(const move_only& other) = delete;
  move_only& operator=(const move_only& other) = delete;

  move_only(move_only&& other) : _value(other._value) {}
  move_only& operator=(move_only&& other)
  {
    if (&other != this) _value = other._value;
    return *this;
  }

  int value() const { return _value; }
private:
  int _value;
};

BOOST_AUTO_TEST_CASE(push_pop_test) {
  moving_queue<move_only> queue;
  queue.push(move_only(1));
  queue.push(move_only(2));
  queue.push(move_only(3));

  move_only val;

  BOOST_CHECK_EQUAL(queue.size(), 3);
  queue.pop(&val);
  BOOST_CHECK_EQUAL(val.value(), 1);

  BOOST_CHECK_EQUAL(queue.size(), 2);
  queue.pop(&val);
  BOOST_CHECK_EQUAL(val.value(), 2);

  BOOST_CHECK_EQUAL(queue.size(), 1);
  queue.pop(&val);
  BOOST_CHECK_EQUAL(val.value(), 3);
  BOOST_CHECK_EQUAL(queue.size(), 0);
}

BOOST_AUTO_TEST_CASE(queue_pop_empty)
{
  moving_queue<move_only> queue;

  //the pop call on an empty queue should do nothing
  move_only* item = NULL;
  queue.pop(item);
  if (item)
    BOOST_ERROR("item should be null");
}

BOOST_AUTO_TEST_CASE(queue_move_push)
{
  move_only test(1);
  moving_queue<move_only> queue;

  // Contents of string moved into queue
  queue.push(test);
  BOOST_CHECK_EQUAL(test.value(), 0);

  // Contents of queue string moved into passed in string
  move_only item;
  queue.pop(&item);
  BOOST_CHECK_EQUAL(item.value(), 1);
}
