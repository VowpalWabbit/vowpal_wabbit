#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
#   define BOOST_TEST_MODULE Main
#endif

#include <boost/test/unit_test.hpp>
#include <string>
#include <vector>
#include "ds_async_batcher.h"

using namespace decision_service;

//this class simply implement a 'send' method, in order to be used as a template in the async_batcher
class sender {
    public:
        std::vector<std::string> items;
        void send(const std::string& item) {
            items.push_back(item);
        };
};


BOOST_AUTO_TEST_CASE(flush_timeout)
{
    sender s;
    size_t timeout_ms = 100;//set a short timeout
	async_batcher<sender> batcher(s, 262143, timeout_ms, 8192);
 
    //add 2 items in the current batch
    batcher.append("foo");
    batcher.append("bar");

    //wait until the timeout triggers
	std::this_thread::sleep_for(std::chrono::milliseconds(timeout_ms + 10));
    
    //check the batch was sent
    std::string expected = "foo\nbar";
    BOOST_REQUIRE_EQUAL(s.items.size(), 1);
    BOOST_CHECK_EQUAL(s.items.front(), expected);
}

BOOST_AUTO_TEST_CASE(flush_batches)
{
    sender s;
    size_t batch_max_size = 10;//bytes
	async_batcher<sender>* batcher = new async_batcher<sender>(s, batch_max_size);

    //add 2 items in the current batch
    batcher->append("foo");//3 bytes
    batcher->append("bar");//3 bytes
    //the next item is too big to be added to the current batch. It will be added to another batch
    batcher->append("hello");

    delete batcher;//triggers all batch flushing

    BOOST_REQUIRE_EQUAL(s.items.size(), 2);
    auto batch0 = s.items[0];
    auto batch1 = s.items[1];
    BOOST_CHECK_EQUAL(batch0.c_str(), "foo\nbar");
    BOOST_CHECK_EQUAL(batch1.c_str(), "hello");
}

BOOST_AUTO_TEST_CASE(flush_after_deletion)
{
    sender s;
	async_batcher<sender>* batcher = new async_batcher<sender>(s);

    batcher->append("foo");
    batcher->append("bar");

    //batch was not sent yet
    BOOST_CHECK_EQUAL(s.items.size(), 0);

    //batch flush is triggered on delete
    delete batcher;

    std::string expected = "foo\nbar";

    BOOST_REQUIRE_EQUAL(s.items.size(), 1);
    BOOST_CHECK_EQUAL(s.items.front(), expected);
}
