#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
#   define BOOST_TEST_MODULE Main
#endif

#include "ranking_event.h"
#include <boost/test/unit_test.hpp>
#include "ranking_response.h"
#include "utility/data_buffer.h"

using namespace reinforcement_learning;
using namespace std;

BOOST_AUTO_TEST_CASE(serialize_outcome)
{
  const auto uuid = "uuid";
	const auto outcome_data = 1.0;

  utility::data_buffer oss;
  outcome_event::serialize(oss, uuid, outcome_data);
  const auto serialized_str = oss.str();
  const char * expected = R"({"EventId":"uuid","v":1.000000})";

	BOOST_CHECK_EQUAL(serialized_str.c_str(), expected);
}

BOOST_AUTO_TEST_CASE(serialize_empty_outcome)
{
	const auto uuid = "";
	const auto outcome_data = "{}";

  utility::data_buffer oss;
  outcome_event::serialize(oss, uuid, outcome_data);
  const auto serialized = oss.str();
	const auto expected = R"({"EventId":"","v":{}})";

	BOOST_CHECK_EQUAL(serialized.c_str(), expected);
}

BOOST_AUTO_TEST_CASE(serialize_ranking)
{
	const auto uuid = "uuid";
	const auto context = "{context}";
	ranking_response resp;
	resp.push_back(1, 0.8f);
	resp.push_back(0, 0.2f);
	resp.set_model_id("model_id");

  utility::data_buffer oss;
  ranking_event::serialize(oss, uuid, context, resp);
  const std::string serialized = oss.str();
	const auto expected = R"({"Version":"1","EventId":"uuid","a":[2,1],"c":{context},"p":[0.800000,0.200000],"VWState":{"m":"model_id"}})";

	BOOST_CHECK_EQUAL(serialized.c_str(), expected);
}

BOOST_AUTO_TEST_CASE(serialize_empty_ranking)
{
	const auto uuid = "uuid";
	const auto context = "{context}";
	ranking_response ranking;
	ranking.set_model_id("model_id");

  utility::data_buffer oss;
  ranking_event::serialize(oss, uuid, context, ranking);
  const auto serialized = oss.str();
	const auto expected = R"({"Version":"1","EventId":"uuid","a":[],"c":{context},"p":[],"VWState":{"m":"model_id"}})";

	BOOST_CHECK_EQUAL(serialized.c_str(), expected);
}
