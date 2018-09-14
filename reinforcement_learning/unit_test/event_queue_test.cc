#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
#   define BOOST_TEST_MODULE Main
#endif

#include "logger/event_queue.h"
#include <boost/test/unit_test.hpp>

using namespace reinforcement_learning;
using namespace std;

class test_event : public event {
public:
  test_event() {}
  test_event(const string& id) : event(id.c_str()) {}

  test_event(test_event&& other) : event(std::move(other)) {}
  test_event& operator=(test_event&& other)
  {
    if (&other != this) event::operator=(std::move(other));
    return *this;
  }

  bool try_drop(float drop_prob, int _drop_pass) override {
    return _event_id.substr(0, 4) == "drop";
  }

  std::string str() override {
    return _event_id;
  }
};

BOOST_AUTO_TEST_CASE(push_pop_test) {
  event_queue<test_event> queue;
  queue.push(test_event("1"));
  queue.push(test_event("2"));
  queue.push(test_event("3"));

  test_event val;

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
  event_queue<test_event> queue;
  queue.push(test_event("no_drop_1"));
  queue.push(test_event("drop_1"));
  queue.push(test_event("no_drop_2"));
  queue.push(test_event("drop_2"));
  queue.push(test_event("no_drop_3"));

  test_event val;

  BOOST_CHECK_EQUAL(queue.size(), 5);
  queue.prune(1.0);

  queue.pop(&val);
  BOOST_CHECK_EQUAL(val.str(), "no_drop_1");

  queue.pop(&val);
  BOOST_CHECK_EQUAL(val.str(), "no_drop_2");

  queue.pop(&val);
  BOOST_CHECK_EQUAL(val.str(), "no_drop_3");
}

BOOST_AUTO_TEST_CASE(queue_push_pop)
{
  reinforcement_learning::event_queue<test_event> queue;

  //push n elements in the queue
  int n = 10;
  for (int i=0; i<n; ++i)
      queue.push(test_event(std::to_string(i+1)));

  BOOST_CHECK_EQUAL(queue.size(), n);

  //pop front
  test_event item;
  queue.pop(&item);
  BOOST_CHECK_EQUAL(item.str(), std::string("1"));

  //pop all
  while (queue.size()>0)
      queue.pop(&item);

  //check last item
  BOOST_CHECK_EQUAL(item.str(), std::to_string(n));

  //check queue size
  BOOST_CHECK_EQUAL(queue.size(), 0);
}

BOOST_AUTO_TEST_CASE(queue_pop_empty)
{
  reinforcement_learning::event_queue<test_event> queue;

  //the pop call on an empty queue should do nothing
  test_event* item = NULL;
  queue.pop(item);
  if (item)
      BOOST_ERROR("item should be null");
}

BOOST_AUTO_TEST_CASE(queue_move_push)
{
  test_event test("hello");
  reinforcement_learning::event_queue<test_event> queue;

  // Contents of string moved into queue
  queue.push(test);
  BOOST_CHECK_EQUAL(test.str(), "");

  // Contents of queue string moved into passed in string
  test_event item;
  queue.pop(&item);
  BOOST_CHECK_EQUAL(item.str(), "hello");
}
