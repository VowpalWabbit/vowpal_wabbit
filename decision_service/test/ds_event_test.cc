#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
#   define BOOST_TEST_MODULE Main
#endif

#include "../src/ds_event.h"
#include <boost/test/unit_test.hpp>

using namespace decision_service;

BOOST_AUTO_TEST_CASE(serialize_outcome)
{
	const char* uuid = "uuid";
	const char* outcome_data = "1.0";

	outcome_event evt(uuid, outcome_data);

	std::string serialized = evt.serialize();
	std::string expected = R"({"EventId":"uuid","v":"1.0"})";

	BOOST_CHECK_EQUAL(serialized, expected);
}

BOOST_AUTO_TEST_CASE(serialize_empty_outcome)
{
	const char* uuid = "";
	const char* outcome_data = "";

	outcome_event evt(uuid, outcome_data);

	std::string serialized = evt.serialize();
	std::string expected = R"({"EventId":"","v":""})";

	BOOST_CHECK_EQUAL(serialized, expected);
}

BOOST_AUTO_TEST_CASE(serialize_ranking)
{
	const char* uuid = "uuid";
	const char* context = "{context}";
	std::vector<std::pair<int, float>> ranking;
	ranking.push_back(std::pair<int, float>(2, 0.8f));
	ranking.push_back(std::pair<int, float>(1, 0.2f));
	std::string model_id = "model_id";

	ranking_event evt(uuid, context, ranking, model_id);

	std::string serialized = evt.serialize();
	std::string expected = R"({"Version":"1","EventId":"uuid","a":[2,1],"c":{context},"p":[0.8,0.2],"VWState":{"m":"model_id"}})";

	BOOST_CHECK_EQUAL(serialized, expected);
}

BOOST_AUTO_TEST_CASE(serialize_empty_ranking)
{
	const char* uuid = "uuid";
	const char* context = "{context}";
	std::vector<std::pair<int, float>> ranking;
	std::string model_id = "model_id";

	ranking_event evt(uuid, context, ranking, model_id);

	std::string serialized = evt.serialize();
	std::string expected = R"({"Version":"1","EventId":"uuid","a":[],"c":{context},"p":[],"VWState":{"m":"model_id"}})";

	BOOST_CHECK_EQUAL(serialized, expected);
}
