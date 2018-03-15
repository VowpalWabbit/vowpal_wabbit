#include "ds_event.h"
#include "ds_driver.h"
#include "ds_eventhub.h"

#include <string>
#include <vector>

using namespace decision_service;
using namespace std;

int main()
{
	//DRIVER
	//dummy configuration
	auto dummy_config = R"({"eventhub_interaction_url":"http://localhost:8080", "eventhub_observation_url":"http://localhost:8080", "app_id":"my_app_id","model_url":"http://azure.com/blob","model_refresh_period_ms":300000})";

	//create a ds driver, with the dummy configuration
	auto* ds = driver::create(dummy_config);

	//request the ds to choose the top action
	auto ranking_response = ds->choose_ranking(R"({"User":{"_age":22},"Geo":{"country":"United States","state":"California","city":"Anaheim"},"_multi":[{"_tag":"cmplx$http://www.complex.com/style/2017/06/kid-puts-together-hypebeast-pop-up-book-for-art-class"},{"_tag":"cmplx$http://www.complex.com/sports/2017/06/floyd-mayweather-will-beat-conor-mcgregor"}]})");

	//do something with the top_action
	std::cout << "top_action_id = " << ranking_response.top_action_id() << std::endl;

	//report a reward later, providing the uuid
	ds->report_outcome(ranking_response.uuid().c_str(), 1.0f);

	//request another top action
	//ranking_response = ds->choose_ranking(R"({"User":{"_age":22},"Geo":{"country":"United States","state":"California","city":"Anaheim"},"_multi":[{"_tag":"cmplx$http://www.complex.com/style/2017/06/kid-puts-together-hypebeast-pop-up-book-for-art-class"},{"_tag":"cmplx$http://www.complex.com/sports/2017/06/floyd-mayweather-will-beat-conor-mcgregor"}]})");
	//std::cout << "top_action_id = " << ranking_response.top_action_id() << std::endl;

	//ds->report_outcome(ranking_response.uuid().c_str(), 0.3f);

	ds->destroy();


	return 0;
}
	//OUTCOME LOG

	//string uuid = "b94a280e32024acb9a4fa12b058157d3";
	//string outcome_data = "1.0";
	//string context = R"({"User":{"_age":22},"Geo":{"country":"United States","state":"California","city":"Anaheim"},"_multi":[{"_tag":"cmplx$http://www.complex.com/style/2017/06/kid-puts-together-hypebeast-pop-up-book-for-art-class"},{"_tag":"cmplx$http://www.complex.com/sports/2017/06/floyd-mayweather-will-beat-conor-mcgregor"}]})";
	//vector<pair<int, float>> rank;
	//rank.push_back(pair<int, float>(1, 0.5f));
	//string model_id = "680ec362b798463eaf64489efaa0d7b1/eaf64489efaa0d7b1680ec362b798463";

	//configuration config(R"({"eh_url":"https://ingest-x2bw4dlnkv63q.servicebus.windows.net/observation/messages?timeout=60&api-version=2014-01", "eh_host":"ingest-x2bw4dlnkv63q.servicebus.windows.net", "eh_name":"observation", "eh_key_name":"RootManageSharedAccessKey", "eh_key_value":"2MNeQafvOsmV9jgbUOtx4I1KBAtKUEXqU2e6Om8M/n4="})");
	//configuration config(R"({"eventhub_interaction_url":"http://localhost:8080", "eventhub_observation_url":"http://localhost:8080"})");

	//eventhub eh(&config, config.eventhub_interaction_url(), config.eventhub_interaction_name());

	//ranking_event revt(uuid, context, rank, model_id);
	//outcome_event oevt(uuid, outcome_data);