#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
#   define BOOST_TEST_MODULE Main
#endif

#include "http_server/stdafx.h"
#include "ds_eventhub_client.h"
#include "http_server/http_server.h"

#include <boost/test/unit_test.hpp>

using namespace web;
using namespace http;
using namespace utility;
using namespace http::experimental::listener;


using namespace decision_service;


std::unique_ptr<http_server> g_http;

void on_initialize(const string_t& address)
{
    // Build our listener's URI from the configured address and the hard-coded path "MyServer/Action"
    uri_builder uri(address);

    auto addr = uri.to_uri().to_string();
	g_http = std::unique_ptr<http_server>(new http_server(addr));
	g_http->open().wait();

    return;
}

void on_shutdown()
{
	g_http->close().wait();
    return;
}


BOOST_AUTO_TEST_CASE(send_something)
{
	//start a http server that will receive events sent from the eventhub_client
    on_initialize(U("http://localhost:8080"));

	//create a client
	eventhub_client eh("localhost:8080", "", "", "");

	//sedn events
	eh.send("message 1");
	eh.send("message 2");

	//stop the http server
	on_shutdown();
}
