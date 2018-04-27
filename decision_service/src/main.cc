#include "ds_event.h"
#include "ds_driver.h"

#include <string>
#include <vector>
#include <iostream>

using namespace decision_service;


int main()
{
	//dummy configuration
	auto dummy_config = R"({"eventhub_host":"localhost:8080"})";

	//create a ds driver, with the dummy configuration
	auto* ds = driver::create(dummy_config);

	//request the ds to choose the top action
	auto ranking_response = ds->choose_ranking(R"(1)", R"({"User":{"_age":22},"Geo":{"country":"United States","state":"California","city":"Anaheim"},"_multi":[{"_tag":"cmplx$http://www.complex.com/style/2017/06/kid-puts-together-hypebeast-pop-up-book-for-art-class"},{"_tag":"cmplx$http://www.complex.com/sports/2017/06/floyd-mayweather-will-beat-conor-mcgregor"}]})");

	/* do something with the top_action */

	//report the reward
	ds->report_outcome(ranking_response.uuid().c_str(), 1.0f);

	//request another ranking
	ranking_response = ds->choose_ranking(R"(2)", R"({"User":{"_age":22},"Geo":{"country":"United States","state":"California","city":"Anaheim"},"_multi":[{"_tag":"cmplx$http://www.complex.com/style/2017/06/kid-puts-together-hypebeast-pop-up-book-for-art-class"},{"_tag":"cmplx$http://www.complex.com/sports/2017/06/floyd-mayweather-will-beat-conor-mcgregor"}]})");
	
	//send reward
	ds->report_outcome(ranking_response.uuid().c_str(), 2.0f);

	ds->destroy();


	return 0;
}

	// //send to eventhub test
	// eventhub eh("localhost:8080", "key-name", "key", "name");

	// int n=5;
	// while (n-->0) {
	// 	eh.send(std::to_string(5-n));
	// }
