#include "../src/ds_event.h"

#define BOOST_TEST_MODULE ds_event
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(serialize_outcome)
{
	decision_service::outcome_event evt("b94a280e32024acb9a4fa12b058157d3", "1.0");

	std::string serialized = evt.serialize();
	std::string expected = R"("{"EventId":"b94a280e32024acb9a4fa12b058157d3","v":"1.0"})";

	BOOST_CHECK_EQUAL(serialized, expected);
}
