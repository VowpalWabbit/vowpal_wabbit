#pragma once

#include <string>

namespace decision_service {

	class configuration {
	public:
		//app name
		const std::string& app_id() const;

		//model settings
		const std::string& model_url() const;
		const int model_refresh_period_ms() const;

		//logging settings
		const std::string& eventhub_interaction_url() const;
		const std::string& eventhub_interaction_name() const;
		const std::string& eventhub_observation_url() const;
		const std::string& eventhub_observation_name() const;
		const std::string& eventhub_host() const;
		const std::string& eventhub_key_name() const;
		const std::string& eventhub_key_value() const;
		const size_t batch_max_size() const;
		const size_t queue_max_size() const;
		const int batch_timeout_ms() const;

		configuration();
		configuration(const std::string&);//deserialize the json string

	private:
		std::string _app_id;
		std::string _model_url;
		int _model_refresh_period_ms;
		std::string _eventhub_interaction_url;
		std::string _eventhub_interaction_name;
		std::string _eventhub_observation_url;
		std::string _eventhub_observation_name;
		std::string _eventhub_host;
		std::string _eventhub_key_name;
		std::string _eventhub_key_value;
		size_t _batch_max_size;
		size_t _queue_max_size;
		int _batch_timeout_ms;
	};
}