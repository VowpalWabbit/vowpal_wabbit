#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
#   define BOOST_TEST_MODULE Main
#endif

#include "http_server/stdafx.h"
#include "logger/eventhub_client.h"
#include "http_server/http_server.h"

#include <boost/test/unit_test.hpp>

using namespace web;
using namespace http;
using namespace utility;
using namespace http::experimental::listener;


using namespace reinforcement_learning;


BOOST_AUTO_TEST_CASE(send_something)
{
	//start a http server that will receive events sent from the eventhub_client
	http_helper http_server;
	http_server.on_initialize(U("http://localhost:8080"));

	//create a client
	eventhub_client eh("localhost:8080", "", "", "");

	//sedn events
	eh.send("message 1");
	eh.send("message 2");

	//stop the http server
	http_server.on_shutdown();
}
