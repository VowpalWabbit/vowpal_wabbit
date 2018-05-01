#include "ds_driver.h"

#include <vector>
#include "../include/ds_utility.h"

using namespace decision_service;
using namespace decision_service::utility;
using namespace decision_service::utility::config;

config_collection load_config();

int main_driver()
{
  auto const config = load_config();

	// Create a ds driver, and initialize with configuration
  driver ds(config);

	// Use ds to choose the top action
	auto ranking_response = ds.choose_ranking(R"(uuid_1)", R"({"User":{"_age":22},"Geo":{"country":"United States","state":"California","city":"Anaheim"},"_multi":[{"_tag":"cmplx$http://www.complex.com/style/2017/06/kid-puts-together-hypebeast-pop-up-book-for-art-class"},{"_tag":"cmplx$http://www.complex.com/sports/2017/06/floyd-mayweather-will-beat-conor-mcgregor"}]})");

	/* do something with the top_action */

	// Report the reward to ds
	ds.report_outcome(ranking_response.uuid().c_str(), 1.0f);

	// Request another ranking
	ranking_response = ds.choose_ranking(R"(uuid_2)", R"({"User":{"_age":22},"Geo":{"country":"United States","state":"California","city":"Anaheim"},"_multi":[{"_tag":"cmplx$http://www.complex.com/style/2017/06/kid-puts-together-hypebeast-pop-up-book-for-art-class"},{"_tag":"cmplx$http://www.complex.com/sports/2017/06/floyd-mayweather-will-beat-conor-mcgregor"}]})");
	
	// Send reward
	ds.report_outcome(ranking_response.uuid().c_str(), 2.0f);

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