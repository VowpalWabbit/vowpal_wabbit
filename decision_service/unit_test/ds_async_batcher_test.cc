#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
#   define BOOST_TEST_MODULE Main
#endif

#include <boost/test/unit_test.hpp>
#include <string>
#include <vector>
#include "logger/ds_async_batcher.h"

using namespace decision_service;

//this class simply implement a 'send' method, in order to be used as a template in the async_batcher
class sender {
    public:
        std::vector<std::string> items;
        int send(const std::string& item, api_status* s = nullptr) {
            items.push_back(item);
            return error_code::success;
        };
};

void expect_no_error(const api_status& s, void* cntxt)
{
  BOOST_ASSERT(s.get_error_code() == error_code::success);
  BOOST_FAIL("Should not get background error notifications");
}

//test the flush mecanism based on a timer
BOOST_AUTO_TEST_CASE(flush_timeout)
{
    sender s;
    size_t timeout_ms = 100;//set a short timeout
    error_callback_fn error_fn(expect_no_error, nullptr);
	  async_batcher<sender> batcher(s, &error_fn,262143, timeout_ms, 8192);
 
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

//test that the batcher split batches as expected
BOOST_AUTO_TEST_CASE(flush_batches)
{
    sender s;
    size_t batch_max_size = 10;//bytes
    error_callback_fn error_fn(expect_no_error, nullptr);
    async_batcher<sender>* batcher = new async_batcher<sender>(s, &error_fn, batch_max_size);

    //add 2 items in the current batch
    batcher->append("foo");//3 bytes
    batcher->append("bar");//3 bytes

    //the next item cannot be added in the current batch. Otherwise the batch size would exceed 'batch_max_size'.
	//so it will be added in a new batch
    batcher->append("hello");

	std::string expected_batch_0 = "foo\nbar";
	std::string expected_batch_1 = "hello";

	delete batcher;//flush force

    BOOST_REQUIRE_EQUAL(s.items.size(), 2);
    auto batch_0 = s.items[0];
    auto batch_1 = s.items[1];

	BOOST_CHECK_EQUAL(batch_0, expected_batch_0);
    BOOST_CHECK_EQUAL(batch_1, expected_batch_1);
}

//test that the batcher flushes everything before deletion
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


//test that events are dropped if the queue max capacity is reached
BOOST_AUTO_TEST_CASE(queue_overflow_drop_event)
{
	sender s;
	size_t timeout_ms = 100;
	size_t queue_max_size = 2;
  error_callback_fn error_fn(expect_no_error, nullptr);
  async_batcher<sender>* batcher = new async_batcher<sender>(s, &error_fn,262143, timeout_ms, queue_max_size);

	BOOST_CHECK_EQUAL(batcher->append("1"), error_code::success);
	BOOST_CHECK_EQUAL(batcher->append("2"), error_code::success);
	BOOST_CHECK_EQUAL(batcher->append("3"), error_code::background_queue_overflow);
	
	//'3' was dropped because of the overflow
	std::string expected_batch = "1\n2";

	delete batcher;

	BOOST_REQUIRE_EQUAL(s.items.size(), 1);
	BOOST_CHECK_EQUAL(s.items.front(), expected_batch);
}

//test that status_api is correctly set when the queue_overflow error happens
BOOST_AUTO_TEST_CASE(queue_overflow_return_error)
{
	sender s;
	size_t queue_max_size = 2;
  error_callback_fn error_fn(expect_no_error, nullptr);
  async_batcher<sender> batcher(s, &error_fn ,262143, 1000, queue_max_size);
	
	//pass the status to each call, then check its content
	api_status status;

	//adding 2 elements of ok
	BOOST_CHECK_EQUAL(batcher.append("1", &status), error_code::success);
	BOOST_CHECK_EQUAL(batcher.append("2", &status), error_code::success);
	
	//the batcher will try to exceed its queue capacity
	BOOST_CHECK_EQUAL(batcher.append("3", &status), error_code::background_queue_overflow);
	
	//verify that the error status is correct
	BOOST_CHECK_EQUAL(status.get_error_code(), error_code::background_queue_overflow);
	BOOST_CHECK_EQUAL(status.get_error_msg().c_str(), "dropped event: 3");
}
