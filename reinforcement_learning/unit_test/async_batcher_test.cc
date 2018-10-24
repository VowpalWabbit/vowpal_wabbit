#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
#   define BOOST_TEST_MODULE Main
#endif

#include <boost/test/unit_test.hpp>
#include <string>
#include <vector>
#include "logger/async_batcher.h"
#include "utility/data_buffer.h"
#include "err_constants.h"

using namespace reinforcement_learning;

//this class simply implement a 'send' method, in order to be used as a template in the async_batcher
class sender : public i_sender {
public:
  std::vector<unsigned char>& items;
  sender(std::vector<unsigned char>& _items) : items(_items) {}

  virtual int init(api_status* s) override {
    return 0;
  }

  virtual int v_send(const std::string &item, api_status* s = nullptr) override{
    for (auto ch : item) {
      items.push_back(ch);
    }
    return error_code::success;
  };

  virtual int v_send(const std::vector<unsigned char> &data, api_status* status) override {
    for (auto ch : data) {
      items.push_back(ch);
    }
    return error_code::success;
  };
};

class test_undroppable_event : public event {
public:
  test_undroppable_event() {}
  test_undroppable_event(const std::string& id) : event(id.c_str()) {}

  test_undroppable_event(test_undroppable_event&& other) : event(std::move(other)) {}
  test_undroppable_event& operator=(test_undroppable_event&& other)
  {
    if (&other != this) event::operator=(std::move(other));
    return *this;
  }

  bool try_drop(float drop_prob, int _drop_pass) override {
    return false;
  }
  
  std::string get_event_id() {
    return _event_id;
  }
};

void expect_no_error(const api_status& s, void* cntxt)
{
  BOOST_ASSERT(s.get_error_code() == error_code::success);
  BOOST_FAIL("Should not get background error notifications");
}

//test the flush mecanism based on a timer
BOOST_AUTO_TEST_CASE(flush_timeout)
{
  std::vector<unsigned char> items;
  auto s = new sender(items);

  size_t timeout_ms = 100;//set a short timeout
  error_callback_fn error_fn(expect_no_error, nullptr);
  utility::watchdog watchdog;
  async_batcher<test_undroppable_event> batcher(s, watchdog, &error_fn,262143, timeout_ms, 8192);
  batcher.init(nullptr);

  //add 2 items in the current batch
  batcher.append(test_undroppable_event("foo"));
  batcher.append(test_undroppable_event("bar"));

  //wait until the timeout triggers
  std::this_thread::sleep_for(std::chrono::milliseconds(timeout_ms + 30));

  //check the batch was sent
  std::string expected = "foobar";
  BOOST_REQUIRE_EQUAL(items.size(), 6);
  std::string result;
  for (auto ch : items) {
    result.push_back((char)ch);
  }
  BOOST_CHECK_EQUAL(result, expected);
}

//test that the batcher flushes everything before deletion
BOOST_AUTO_TEST_CASE(flush_after_deletion)
{
  std::vector<unsigned char> items;
  auto s = new sender(items);
  utility::watchdog watchdog;
  async_batcher<test_undroppable_event>* batcher = new async_batcher<test_undroppable_event>(s, watchdog);
  batcher->init(nullptr);

  batcher->append(test_undroppable_event("foo"));
  batcher->append(test_undroppable_event("bar"));

  //batch was not sent yet
  BOOST_CHECK_EQUAL(items.size(), 0);

  //batch flush is triggered on delete
  delete batcher;
  //check the batch was sent
  std::string expected = "foobar";
  BOOST_REQUIRE_EQUAL(items.size(), 6);
  std::string result;
  for (auto ch : items) {
    result.push_back((char)ch);
  }
  BOOST_CHECK_EQUAL(result, expected);
}

//test that events are dropped if the queue max capacity is reached
/*BOOST_AUTO_TEST_CASE(queue_overflow_drop_event)
{
  std::vector<std::string> items;
  auto s = new sender(items);
  size_t timeout_ms = 100;
  size_t queue_max_size = 2;
  error_callback_fn error_fn(expect_no_error, nullptr);
  utility::watchdog watchdog;
  async_batcher<test_event>* batcher = new async_batcher<test_event>(s, watchdog, &error_fn,262143, timeout_ms, queue_max_size);

  BOOST_CHECK_EQUAL(batcher->append(test_event("1")), error_code::success);
  BOOST_CHECK_EQUAL(batcher->append(test_event("2")), error_code::success);
  BOOST_CHECK_EQUAL(batcher->append(test_event("3")), error_code::background_queue_overflow);

  //'3' was dropped because of the overflow
  std::string expected_batch = "1\n2";

  delete batcher;

  BOOST_REQUIRE_EQUAL(items.size(), 1);
  BOOST_CHECK_EQUAL(items.front(), expected_batch);
}*/

//test that status_api is correctly set when the queue_overflow error happens
/*BOOST_AUTO_TEST_CASE(queue_overflow_return_error)
{
  std::vector<std::string> items;
  auto s = new sender(items);
  size_t queue_max_size = 2;
  error_callback_fn error_fn(expect_no_error, nullptr);
  utility::watchdog watchdog;
  async_batcher<test_event> batcher(s, watchdog, &error_fn ,262143, 1000, queue_max_size);

  //pass the status to each call, then check its content
  api_status status;

  //adding 2 elements of ok
  BOOST_CHECK_EQUAL(batcher.append(test_event("1"), &status), error_code::success);
  BOOST_CHECK_EQUAL(batcher.append(test_event("2"), &status), error_code::success);

  //the batcher will try to exceed its queue capacity
  BOOST_CHECK_EQUAL(batcher.append(test_event("3"), &status), error_code::background_queue_overflow);

  //verify that the error status is correct
  BOOST_CHECK_EQUAL(status.get_error_code(), error_code::background_queue_overflow);
}*/
