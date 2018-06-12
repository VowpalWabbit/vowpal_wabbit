#include "personalization.h"

#include <iostream>

using namespace personalization;
using namespace personalization::utility;
using namespace personalization::utility::config;

config_collection load_config();
void display_response(const ranking_response&);

void error_handler(const api_status& error, void* user_context)
{
  std::cout << "An error happened in the background thread. Code:" << error.get_error_code() 
    << " Details:" << error.get_error_msg() << std::endl;
}

int main()
{
	const auto config = init_from_json(R"({"eventhub_host":"localhost:8080"})");

	auto error_cntxt = 1;
	// Create a ds live_model, and initialize with configuration
	live_model model(config, error_handler, (void*)(&error_cntxt));

  api_status status;  //optional, can be omitted
  model.init(&status);

	//create response and api_status object, that will be passed to the ds live_model
	ranking_response response;

	// Use ds to choose the top action
	const auto uuid = R"(uuid_1)";
	const auto context = R"({"User":{"_age":22},"Geo":{"country":"United States","state":"California","city":"Anaheim"},"_multi":[{"_tag":"cmplx$http://www.complex.com/style/2017/06/kid-puts-together-hypebeast-pop-up-book-for-art-class"},{"_tag":"cmplx$http://www.complex.com/sports/2017/06/floyd-mayweather-will-beat-conor-mcgregor"}]})";
	auto success = model.choose_rank(uuid, context, response, &status);
	
	if (success != 0)
	{	
		std::cout << "an error happened with code: " << success << std::endl;
		std::cout << "status error code: " << status.get_error_code() << std::endl;
		std::cout << "status error msg : " << status.get_error_msg() << std::endl;
	}

	/* do something with the top_action */
	display_response(response);

	// Report the reward to ds
	success = model.report_outcome(response.get_uuid(), 1.0f, &status);
	if (success != 0) 
	{
		std::cout << "an error happened with code: " << success << std::endl;
		std::cout << "status error code: " << status.get_error_code() << std::endl;
		std::cout << "status error msg : " << status.get_error_msg() << std::endl;
	}

	// Send another reward with an invalid uuid
	const char* invalid_uuid = "";
	success = model.report_outcome(invalid_uuid, "outcome_data", &status);
	if (success != 0)
	{
		std::cout << "an error happened with code: " << success << std::endl;
		std::cout << "status error code: " << status.get_error_code() << std::endl;
		std::cout << "status error msg : " << status.get_error_msg() << std::endl;
	}
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
	fprintf(stdout, "uuid    : %s\n", response.get_uuid());
	fprintf(stdout, "ranking :  ");
	for (auto i : response)
		std::cout << "(" << i.action_id << "," << i.probability << ") ";
	fprintf(stdout, "\n");
  size_t action_id=0;
  if(error_code::success == response.get_choosen_action_id(action_id))
  	fprintf(stdout, "top action id = %d\n", (int)action_id);
  else
    fprintf(stdout, "error choosing action\n");
}
