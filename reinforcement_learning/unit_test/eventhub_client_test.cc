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

//send string
BOOST_AUTO_TEST_CASE(send_string)
{
  //start a http server that will receive events sent from the eventhub_client
  http_helper http_server;
  BOOST_CHECK(http_server.on_initialize(U("http://localhost:8080")));

  //create a client
  eventhub_client eh("localhost:8080", "", "", "", true);

  api_status ret;
  //send events
  BOOST_CHECK_EQUAL(eh.send_string("message 1", &ret),error_code::success);
  BOOST_CHECK_EQUAL(eh.send_string("message 2", &ret), error_code::success);
}

//send bytes
BOOST_AUTO_TEST_CASE(send_bytes)
{
  //start a http server that will receive events sent from the eventhub_client
  http_helper http_server;
  BOOST_CHECK(http_server.on_initialize(U("http://localhost:8080")));

  //create a client
  eventhub_client eh("localhost:8080", "", "", "", true);

  api_status ret;
  std::vector<unsigned char> messages;
  messages.push_back('\11');
  messages.push_back('\12');
  messages.push_back('\13');

  //send events
  BOOST_CHECK_EQUAL(eh.send_byte(messages, &ret), error_code::success);
}