#include "../src/ds_concurrent_queue.h"

#define BOOST_TEST_MODULE ds_concurrent_queue_test
#include <boost/test/unit_test.hpp>


BOOST_AUTO_TEST_CASE(push_pop)
{
    decision_service::concurrent_queue<std::string> queue;

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

BOOST_AUTO_TEST_CASE(pop_empty)
{
	decision_service::concurrent_queue<int> queue;

	//the pop call on an empty queue should do nothing
    int* item = NULL;
	queue.pop(item);
    if (item)
        BOOST_ERROR("item should be null");
}