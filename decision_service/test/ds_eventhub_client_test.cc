#include "../src/ds_eventhub_client.h"

#define BOOST_TEST_MODULE ds_eventhub_test
#include <boost/test/unit_test.hpp>


using namespace decision_service;

BOOST_AUTO_TEST_CASE(send_something)
{
	//require a http server on locahost:8080

	eventhub_client eh("localhost:8080", "", "", "");

	eh.send("message 1");
	eh.send("message 2");

	BOOST_CHECK(true);
}
