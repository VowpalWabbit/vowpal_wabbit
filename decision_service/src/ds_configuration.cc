#include "rapidjson/document.h"

#include "ds_configuration.h"

#include <string>


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
		rapidjson::Document document;
		document.Parse(json.c_str());

		if (document.HasParseError()) {
			throw std::exception();
		}

		//read fields
		if (document.HasMember("app_id")) {
			_app_id = document["app_id"].GetString();
		}
		if (document.HasMember("model_url")) {
			_model_url = document["model_url"].GetString();
		}
		if (document.HasMember("model_refresh_period_ms")) {
			_model_refresh_period_ms = document["model_refresh_period_ms"].GetInt();
		}
		if (document.HasMember("eventhub_interaction_name")) {
			_eventhub_interaction_name = document["eventhub_interaction_name"].GetString();
		}
		if (document.HasMember("eventhub_observation_name")) {
			_eventhub_observation_name = document["eventhub_observation_name"].GetString();
		}
		if (document.HasMember("eventhub_host")) {
			_eventhub_host = document["eventhub_host"].GetString();
		}
		if (document.HasMember("shared_access_key_name")) {
			_shared_access_key_name = document["shared_access_key_name"].GetString();
		}
		if (document.HasMember("shared_access_key")) {
			_shared_access_key = document["shared_access_key"].GetString();
		}
		if (document.HasMember("batch_max_size")) {
			_batch_max_size = document["batch_max_size"].GetInt();
		}
		if (document.HasMember("queue_max_size")) {
			_queue_max_size = document["queue_max_size"].GetInt();
		}
		if (document.HasMember("batch_timeout_ms")) {
			_batch_timeout_ms = document["batch_timeout_mss"].GetInt();
		}
	}
};
