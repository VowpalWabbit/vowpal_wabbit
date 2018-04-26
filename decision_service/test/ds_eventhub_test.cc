#include "../src/ds_eventhub.h"

#define BOOST_TEST_MODULE ds_eventhub_test
#include <boost/test/unit_test.hpp>


using namespace decision_service;

BOOST_AUTO_TEST_CASE(send_something)
{
	//require a http server on locahost:8080

	eventhub eh("http://localhost:8080");

	int n = 3;
	for (int i=1; i<n; ++i)
	{
		std::string msg = "msg:" + std::to_string(i);
		eh.send(msg);
	}

	BOOST_CHECK(true);
}
