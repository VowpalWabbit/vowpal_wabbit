#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
#   define BOOST_TEST_MODULE Main
#endif

#include <boost/test/unit_test.hpp>
#include "logger/moving_queue.h"

/*BOOST_AUTO_TEST_CASE(queue_push_pop)
{
  reinforcement_learning::moving_queue<std::string> queue;

  //push n elements in the queue
  int n = 10;
  for (int i=0; i<n; ++i)
      queue.push(std::to_string(i+1));

  BOOST_CHECK_EQUAL(queue.size(), n);

  //pop front
  std::string item;
  queue.pop(&item);
  BOOST_CHECK_EQUAL(item, std::string("1"));

  //pop all
  while (queue.size()>0)
      queue.pop(&item);

  //check last item
  BOOST_CHECK_EQUAL(item, std::to_string(n));

  //check queue size
  BOOST_CHECK_EQUAL(queue.size(), 0);
}

BOOST_AUTO_TEST_CASE(queue_pop_empty)
{
  reinforcement_learning::moving_queue<int> queue;

  //the pop call on an empty queue should do nothing
  int* item = NULL;
  queue.pop(item);
  if (item)
      BOOST_ERROR("item should be null");
}

BOOST_AUTO_TEST_CASE(queue_move_push)
{
  std::string test("hello");
  reinforcement_learning::moving_queue<std::string> queue;

  // Contents of string moved into queue
  queue.push(test);
  BOOST_CHECK_EQUAL(test, "");

  // Contents of queue string moved into passed in string
  std::string item;
  queue.pop(&item);
  BOOST_CHECK_EQUAL(item, "hello");
}
*/