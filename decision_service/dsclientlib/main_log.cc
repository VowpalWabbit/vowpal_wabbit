#include "ds_async_batcher.h"
#include "ds_eventhub.h"
#include "ds_logger.h"

namespace ds = decision_service;
namespace dsutil = decision_service::utility;

int main_log()
{
	ds::eventhub eh("http://localhost:8080");
	eh.send("plop data 0");

	ds::async_batch<ds::eventhub>* asb_eh = new ds::async_batch<ds::eventhub>(eh);
	std::string str("async_batch data 1");
	asb_eh->append(str);
	str = "async_batch data 2";
	asb_eh->append(str);
	str = "async_batch data 3";
	asb_eh->append(str);
	delete asb_eh;

  	dsutil::config_collection config;
	ds::logger logger(config);

	logger.append_ranking("ranking_data");
	logger.append_outcome("outcome_data");

	//auto ranking_response = ds->choose_ranking(R"({"User":{"_age":22},"Geo":{"country":"United States","state":"California","city":"Anaheim"},"_multi":[{"_tag":"cmplx$http://www.complex.com/style/2017/06/kid-puts-together-hypebeast-pop-up-book-for-art-class"},{"_tag":"cmplx$http://www.complex.com/sports/2017/06/floyd-mayweather-will-beat-conor-mcgregor"}]})");
	//configuration config(R"({"eh_url":"https://ingest-x2bw4dlnkv63q.servicebus.windows.net/observation/messages?timeout=60&api-version=2014-01", "eh_host":"ingest-x2bw4dlnkv63q.servicebus.windows.net", "eh_name":"observation", "eh_key_name":"RootManageSharedAccessKey", "eh_key_value":"2MNeQafvOsmV9jgbUOtx4I1KBAtKUEXqU2e6Om8M/n4="})");

	return 0;
}