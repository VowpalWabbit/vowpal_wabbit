#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
#   define BOOST_TEST_MODULE Main
#endif

#include "http_server/stdafx.h"
#include "logger/eventhub_client.h"
#include "http_server/http_server.h"
#include <boost/test/unit_test.hpp>
#include "err_constants.h"

using namespace web;
using namespace http;
using namespace utility;
using namespace http::experimental::listener;

using namespace reinforcement_learning;

class error_counter {
public:
  void _error_handler(void) {
    _err_count++;
  }

  int _err_count = 0;
};

void error_counter_func(const api_status&, void* counter) {
  static_cast<error_counter*>(counter)->_error_handler();
}

BOOST_AUTO_TEST_CASE(send_something)
{
  //start a http server that will receive events sent from the eventhub_client
  http_helper http_server;
  BOOST_CHECK(http_server.on_initialize(U("http://localhost:8080")));

  //create a client
  eventhub_client eh("localhost:8080", "", "", "", 1, 8, nullptr, nullptr, true);

  api_status ret;

  std::string message1Str("message 1");
  std::vector<unsigned char> message1(message1Str.begin(), message1Str.end());

  std::string message2Str("message 2");
  std::vector<unsigned char> message2(message2Str.begin(), message2Str.end());

  //send events
  BOOST_CHECK_EQUAL(eh.send(std::move(message1), &ret), error_code::success);
  BOOST_CHECK_EQUAL(eh.send(std::move(message2), &ret), error_code::success);
}

BOOST_AUTO_TEST_CASE(retry_http_send_success)
{
  http_helper http_server;
  BOOST_CHECK(http_server.on_initialize(U("http://localhost:8080")));

  int tries = 0;
  int succeed_after_n_tries = 3;
  http_server.set_custom_responder(methods::POST, [&tries, succeed_after_n_tries](const http_request& message) {
    tries++;

    if (tries > succeed_after_n_tries) {
      message.reply(status_codes::Created);
    }
    else {
      message.reply(status_codes::InternalError);
    }
  });

  error_counter counter;
  error_callback_fn error_callback(&error_counter_func, &counter);

  // Use scope to force destructor and therefore flushing of buffers.
  {
    //create a client
    eventhub_client eh("localhost:8080", "", "", "", 1, 8 /* retries */, nullptr, &error_callback, true);

    api_status ret;

    std::string message1Str("message 1");
    std::vector<unsigned char> message1(message1Str.begin(), message1Str.end());
    BOOST_CHECK_EQUAL(eh.send(std::move(message1), &ret), error_code::success);
  }

  // Although it was allowed to retry 8 times, it should stop after succeeding at 4.
  BOOST_CHECK_EQUAL(tries, succeed_after_n_tries + 1);
  BOOST_CHECK_EQUAL(counter._err_count, 0);
}

BOOST_AUTO_TEST_CASE(retry_http_send_fail)
{
  http_helper http_server;
  BOOST_CHECK(http_server.on_initialize(U("http://localhost:8080")));

  const int MAX_RETRIES = 10;

  int tries = 0;
  http_server.set_custom_responder(methods::POST, [&tries](const http_request& message) {
    tries++;
    message.reply(status_codes::InternalError);
  });

  error_counter counter;
  error_callback_fn error_callback(&error_counter_func, &counter);

  // Use scope to force destructor and therefore flushing of buffers.
  {
    //create a client
    eventhub_client eh("localhost:8080", "", "", "", 1, MAX_RETRIES, nullptr, &error_callback, true);

    api_status ret;
    std::string message1Str("message 1");
    std::vector<unsigned char> message1(message1Str.begin(), message1Str.end());
    BOOST_CHECK_EQUAL(eh.send(std::move(message1), &ret), error_code::success);
  }

  BOOST_CHECK_EQUAL(tries, MAX_RETRIES + 1);
  BOOST_CHECK_EQUAL(counter._err_count, 1);
}


BOOST_AUTO_TEST_CASE(http_in_order_after_retry)
{
  http_helper http_server;
  BOOST_CHECK(http_server.on_initialize(U("http://localhost:8080")));

  const int MAX_RETRIES = 10;
  int tries = 0;
  std::vector<std::string> received_messages;
  http_server.set_custom_responder(methods::POST, [&tries, &received_messages](const http_request& message) {
    tries++;

    // Succeed every 4th attempt.
    if (tries >= 4) {
      // extract_string can only be called once on an http_request but we only do it once. Using const cast to avoid having to read out the stream.
      std::string test = const_cast<http_request&>(message).extract_utf8string().get();
      received_messages.push_back(test);
      message.reply(status_codes::Created);
      tries = 0;
    }
    else {
      message.reply(status_codes::InternalError);
    }
  });

  error_counter counter;
  error_callback_fn error_callback(&error_counter_func, &counter);

  // Use scope to force destructor and therefore flushing of buffers.
  {
    //create a client
    eventhub_client eh("localhost:8080", "", "", "", 1, MAX_RETRIES, nullptr, &error_callback, true);

    api_status ret;
    std::string message1Str("message 1");
    std::vector<unsigned char> message1(message1Str.begin(), message1Str.end());
    BOOST_CHECK_EQUAL(eh.send(std::move(message1), &ret), error_code::success);

    std::string message2Str("message 2");
    std::vector<unsigned char> message2(message2Str.begin(), message2Str.end());
    BOOST_CHECK_EQUAL(eh.send(std::move(message2), &ret), error_code::success);

    std::string message3Str("message 3");
    std::vector<unsigned char> message3(message3Str.begin(), message3Str.end());
    BOOST_CHECK_EQUAL(eh.send(std::move(message3), &ret), error_code::success);

    std::string message4Str("message 4");
    std::vector<unsigned char> message4(message4Str.begin(), message4Str.end());
    BOOST_CHECK_EQUAL(eh.send(std::move(message4), &ret), error_code::success);

    std::string message5Str("message 5");
    std::vector<unsigned char> message5(message5Str.begin(), message5Str.end());
    BOOST_CHECK_EQUAL(eh.send(std::move(message5), &ret), error_code::success);
  }

  BOOST_CHECK_EQUAL(received_messages[0], "message 1");
  BOOST_CHECK_EQUAL(received_messages[1], "message 2");
  BOOST_CHECK_EQUAL(received_messages[2], "message 3");
  BOOST_CHECK_EQUAL(received_messages[3], "message 4");
  BOOST_CHECK_EQUAL(received_messages[4], "message 5");
  BOOST_CHECK_EQUAL(counter._err_count, 0);
}
