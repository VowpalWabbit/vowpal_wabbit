#include "ds_configuration.h"

#include "cpprest/json.h"

#include <string>

using namespace utility;     // Common utilities like string conversions
using namespace web;

namespace decision_service {

	const std::string & configuration::app_id() const {
		return _app_id;
	}

	const std::string & configuration::model_url() const {
		return _model_url;
	}

	const int configuration::model_refresh_period_ms() const {
		return _model_refresh_period_ms;
	}

	const size_t configuration::batch_max_size() const {
		return _batch_max_size;
	}

	const int configuration::batch_timeout_ms() const {
		return _batch_timeout_ms;
	}

	const size_t configuration::queue_max_size() const {
		return _queue_max_size;
	}

	const std::string & configuration::eventhub_host() const {
		return _eventhub_host;
	}

	const std::string & configuration::shared_access_key_name() const {
		return _shared_access_key_name;
	}

	const std::string & configuration::shared_access_key() const {
		return _shared_access_key;
	}
	const std::string & configuration::eventhub_interaction_name() const {
		return _eventhub_interaction_name;
	}

	const std::string & configuration::eventhub_observation_name() const {
		return _eventhub_observation_name;
	}

	//default values
	configuration::configuration()
		: _app_id(""),
		_model_url(""),
		_model_refresh_period_ms(1000 * 60 * 5), //5 minutes
		_eventhub_interaction_name("interaction"),
		_eventhub_observation_name("observation"),
		_eventhub_host("localhost:8080"),
		_shared_access_key_name(""),
		_shared_access_key(""),
		_batch_max_size(8 * 1024),
		_queue_max_size(10000),
		_batch_timeout_ms(1000 * 2) //2 seconds
	{}

	//deserialize the json string
	configuration::configuration(const std::string& json)
		: configuration()//set default value
	{
		json::value obj = json::value::parse(conversions::to_string_t(json));

		if (obj.has_field(U("app_id")))
			_app_id = conversions::to_utf8string(obj.at(U("app_id")).as_string());
		if (obj.has_field(U("model_url")))
			_model_url = conversions::to_utf8string(obj.at(U("model_url")).as_string());
		if (obj.has_field(U("model_refresh_period_ms")))
			_model_refresh_period_ms = obj.at(U("appmodel_refresh_period_ms_id")).as_integer();
		if (obj.has_field(U("eventhub_interaction_name")))
			_eventhub_interaction_name = conversions::to_utf8string(obj.at(U("eventhub_interaction_name")).as_string());
		if (obj.has_field(U("eventhub_observation_name")))
			_eventhub_observation_name = conversions::to_utf8string(obj.at(U("eventhub_observation_name")).as_string());
		if (obj.has_field(U("eventhub_host")))
			_eventhub_host = conversions::to_utf8string(obj.at(U("eventhub_host")).as_string());
		if (obj.has_field(U("shared_access_key_name")))
			_shared_access_key_name = conversions::to_utf8string(obj.at(U("shared_access_key_name")).as_string());
		if (obj.has_field(U("shared_access_key")))
			_shared_access_key = conversions::to_utf8string(obj.at(U("shared_access_key")).as_string());
		if (obj.has_field(U("batch_max_size")))
			_batch_max_size = obj.at(U("batch_max_size")).as_integer();
		if (obj.has_field(U("queue_max_size")))
			_queue_max_size = obj.at(U("queue_max_size")).as_integer();
		if (obj.has_field(U("batch_timeout_ms")))
			_batch_timeout_ms = obj.at(U("batch_timeout_ms")).as_integer();
	}
};
