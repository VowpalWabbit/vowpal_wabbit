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

  virtual int v_send(std::vector<unsigned char> &&data, api_status* status) override {
    items.insert(items.end(), data.begin(), data.end());
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

  void serialize(utility::data_buffer& buffer) {}
};

class test_droppable_event : public event {
public:
  test_droppable_event() {}
  test_droppable_event(const std::string& id) : event(id.c_str()) {}

  test_droppable_event(test_droppable_event&& other) : event(std::move(other)) {}
  test_droppable_event& operator=(test_droppable_event&& other)
  {
    if (&other != this) event::operator=(std::move(other));
    return *this;
  }

  bool try_drop(float drop_prob, int _drop_pass) override {
    return true;
  }

  void serialize(utility::data_buffer& buffer) {}
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
  utility::watchdog watchdog(nullptr);
  async_batcher<test_undroppable_event> batcher(s, watchdog, &error_fn, 262143, timeout_ms, 8192);
  batcher.init(nullptr);

  //add 2 items in the current batch
  std::string foo("foo");
  std::string bar("bar");
  batcher.append(test_undroppable_event(foo));
  batcher.append(test_undroppable_event(bar));

  //wait until the timeout triggers
  std::this_thread::sleep_for(std::chrono::milliseconds(timeout_ms + 10));

  //check the buffer was sent
  BOOST_REQUIRE_EQUAL(items.size(), foo.length() + bar.length());
  std::string expected = foo + bar;
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
  utility::watchdog watchdog(nullptr);
  async_batcher<test_undroppable_event>* batcher = new async_batcher<test_undroppable_event>(s, watchdog);
  batcher->init(nullptr);

  std::string foo("foo");
  std::string bar("bar");
  batcher->append(test_undroppable_event(foo));
  batcher->append(test_undroppable_event(bar));

  //batch was not sent yet
  BOOST_CHECK_EQUAL(items.size(), 0);

  //batch flush is triggered on delete
  delete batcher;
  //check the buffer was sent
  BOOST_REQUIRE_EQUAL(items.size(), foo.length() + bar.length());

  std::string expected = foo + bar;
  std::string result;
  for (auto ch : items) {
    result.push_back((char)ch);
  }
  BOOST_CHECK_EQUAL(result, expected);
}

//test that events are not dropped using the queue_dropping_disable option, even if the queue max capacity is reached
BOOST_AUTO_TEST_CASE(queue_overflow_do_not_drop_event)
{
  std::vector<unsigned char> items;
  auto s = new sender(items);
  size_t timeout_ms = 100;
  size_t queue_max_size = 3;
  queue_mode_enum queue_mode = BLOCK;
  error_callback_fn error_fn(expect_no_error, nullptr);
  utility::watchdog watchdog(nullptr);
  async_batcher<test_droppable_event>* batcher = new async_batcher<test_droppable_event>(s, watchdog, &error_fn, 262143, timeout_ms, queue_max_size, queue_mode);
  batcher->init(nullptr);

  int n = 10;
  for (int i = 0; i < n; ++i) {
    batcher->append(test_droppable_event(std::to_string(i)));
  }

  //triggers a final flush
  delete batcher;

  //all batches were sent. Check that no event was dropped
  std::string expected = "0123456789";
  BOOST_REQUIRE(items.size() > 0);

  std::string actual;
  for (int i = 0; i < items.size(); i++) {
    actual.push_back(items[i]);
  }
  BOOST_CHECK_EQUAL(expected, actual);
}

BOOST_AUTO_TEST_CASE(convert_to_queue_mode_enum) {
  BOOST_CHECK_EQUAL(DROP, to_queue_mode_enum("DROP"));
  BOOST_CHECK_EQUAL(BLOCK, to_queue_mode_enum("BLOCK"));

  //default is DROP
  BOOST_CHECK_EQUAL(DROP, to_queue_mode_enum("something_else"));
}
