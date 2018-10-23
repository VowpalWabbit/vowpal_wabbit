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

BOOST_AUTO_TEST_CASE(send_something)
{
  //start a http server that will receive events sent from the eventhub_client
  http_helper http_server;
  BOOST_CHECK(http_server.on_initialize(U("http://localhost:8080")));

  //create a client
  eventhub_client eh("localhost:8080", "", "", "", 1, 1, nullptr, nullptr, true);

  api_status ret;
  //send events
  BOOST_CHECK_EQUAL(eh.send("message 1", &ret),error_code::success);
  BOOST_CHECK_EQUAL(eh.send("message 2", &ret), error_code::success);
}

BOOST_AUTO_TEST_CASE(retry_http_send_success)
{

}

BOOST_AUTO_TEST_CASE(retry_http_send_fail)
{

}

BOOST_AUTO_TEST_CASE(move_assign_http_request_task)
{

}
