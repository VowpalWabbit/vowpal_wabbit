#include "ds_utility.h"
#include "cpprest/json.h"

using namespace web;
using namespace utility::conversions; // string conversions utilities

namespace decision_service { namespace utility { namespace config {
  std::string load_config_json()
  { //TODO: Load appid configuration from Azure storage
    //TODO: error handling.  (return code or exception)

    return "";
  }

  config_collection init_from_json(const std::string& config_json)
  {
    //TODO: error handling.  (return code or exception)

	  config_collection cc;

	//TODO enum or something better
	const char* field_names[] = {
		"app_id",
		"model_url",
		"appmodel_refresh_period_ms_id",
		"eventhub_interaction_name",
		"eventhub_observation_name",
		"eventhub_host",
		"shared_access_key_name",
		"shared_access_key",
		"batch_max_size",
		"queue_max_size",
		"batch_timeout_ms"
	};

	json::value obj = json::value::parse(to_string_t(config_json));

	//look for specific field names in the json
	for (auto name : field_names) {
		auto wname = to_string_t(name);//cpp/json works with wstring
		if (obj.has_field(wname)) {
			std::string value = to_utf8string(obj.at(wname).as_string());
			cc.set(name, value.c_str());
		}
	}

    return cc;
  }

}}}
