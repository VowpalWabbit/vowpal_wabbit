#include "ds_driver.h"
#include "ds_utility.h"

#include <vector>
#include <iostream>

using namespace decision_service;
using namespace decision_service::utility;
using namespace decision_service::utility::config;

config_collection load_config();
void display_response(const ranking_response&);

int main()
{
	auto config = init_from_json(R"({"eventhub_host":"localhost:8080"})");

	// Create a ds driver, and initialize with configuration
	driver ds(config);

	//create response and api_status object, that will be passed to the ds driver
	ranking_response response;
	api_status* status = new api_status();//optional, can be omitted

	// Use ds to choose the top action
	const char*  uuid = R"(uuid_1)";
	const char*  context = R"({"User":{"_age":22},"Geo":{"country":"United States","state":"California","city":"Anaheim"},"_multi":[{"_tag":"cmplx$http://www.complex.com/style/2017/06/kid-puts-together-hypebeast-pop-up-book-for-art-class"},{"_tag":"cmplx$http://www.complex.com/sports/2017/06/floyd-mayweather-will-beat-conor-mcgregor"}]})";

	int success = ds.ranking_request(uuid, context, response, status);
	if (success != 0)
		std::cout << "an error happened with code: " << success << std::endl;
	if (status) {
		std::cout << "status error code: " << status->get_error_code() << std::endl;
		std::cout << "status error msg : " << status->get_error_msg() << std::endl;
	}

	/* do something with the top_action */
	display_response(response);

	// Report the reward to ds
	success = ds.report_outcome(response.get_uuid().c_str(), 1.0f, status);
	if (success != 0)
		std::cout << "an error happened with code: " << success << std::endl;
	if (status) {
		std::cout << "status error code: " << status->get_error_code() << std::endl;
		std::cout << "status error msg : " << status->get_error_msg() << std::endl;
	}

	// Send another reward with an invalid uuid
	const char* invalid_uuid = "";
	success = ds.report_outcome(invalid_uuid, "outcome_data", status);
	if (success != 0)
		std::cout << "an error happened with code: " << success << std::endl;
	if (status) {
		std::cout << "status error code: " << status->get_error_code() << std::endl;
		std::cout << "status error msg : " << status->get_error_msg() << std::endl;
	}

	delete status;

	return 0;
}

config_collection load_config()
{
	// Option 1: load configuration from decision service config json
	auto const config_json = load_config_json();
	auto config = init_from_json(config_json);

	// Option 2: set different config values used to initialize ds client library
	config.set("name", "value");
	return config;
}

void display_response(const ranking_response& response)
{
	fprintf(stdout, "uuid    : %s\n", response.get_uuid().c_str());
	fprintf(stdout, "ranking :  ");
	for (auto i : response.get_ranking())
		std::cout << "(" << i.first << "," << i.second << ") ";
	fprintf(stdout, "\n");
	fprintf(stdout, "top action id = %d\n", response.get_top_action_id());
}
