#include "ds_driver.h"
#include "ds_async_batch.h"
#include "ds_event.h"
#include "ds_eventhub.h"

#include <boost/uuid/uuid.hpp>            // uuid class
#include <boost/uuid/uuid_generators.hpp> // generators
#include <boost/uuid/uuid_io.hpp>         // streaming operators etc.


namespace decision_service {

	class driver_impl : public driver {

		configuration _configuration;
		eventhub _outcome_logger;
		async_batch<eventhub> _ranking_logger;

	public:

		ranking_response choose_ranking(const char* uuid, const char* context)
		{
			//check arguments
			if (!uuid || strlen(uuid) == 0)
				throw std::invalid_argument("uuid is null or empty");
			if (!context || strlen(context) == 0)
				throw std::invalid_argument("context is null or empty");

			//get ranking
			//FOR TEST
			std::vector<std::pair<int, float>> ranking;
			ranking.push_back(std::pair<int, float>(2, 0.4f));
			ranking.push_back(std::pair<int, float>(1, 0.3f));
			ranking.push_back(std::pair<int, float>(4, 0.2f));
			ranking.push_back(std::pair<int, float>(3, 0.1f));
			std::string model_id = "model_id";

			//send the serialized event to the backend
			ranking_event evt(uuid, context, ranking, model_id);
			_ranking_logger.append(evt.serialize());

			return ranking_response(uuid, ranking);
		}

		//auto-generate uuid
		ranking_response choose_ranking(const char* context)
		{
			return choose_ranking(boost::uuids::to_string(boost::uuids::random_generator()()).c_str(), context);
		}

		void report_outcome(const char* uuid, const char* outcome_data)
		{
			//check arguments
			if (!uuid || strlen(uuid) == 0)
				throw std::invalid_argument("uuid is empty");
			if (!outcome_data || strlen(outcome_data) == 0)
				throw std::invalid_argument("outcome_data is empty");

			//send the serialized event to the backend
			outcome_event evt(uuid, outcome_data);
			_outcome_logger.send(evt.serialize());
		}

		void report_outcome(const char* uuid, float reward)
		{
			report_outcome(uuid, std::to_string(reward).c_str());
		}

		driver_impl(configuration config)
			: _configuration(config),
			_outcome_logger(config, config.eventhub_observation_url(), config.eventhub_observation_name()),
			_ranking_logger(config, config.eventhub_interaction_url(), config.eventhub_interaction_name())
		{}
	};


	//driver implementation

	driver* driver::create(const char * json_config)
	{
		configuration conf(json_config);
		return static_cast<driver*>(new driver_impl(conf));
	}
	void driver::destroy()
	{
		delete (driver_impl*)this;
	}
	ranking_response driver::choose_ranking(const char * uuid, const char * context)
	{
		return static_cast<driver_impl*>(this)->choose_ranking(uuid, context);
	}
	ranking_response driver::choose_ranking(const char * context)
	{
		return static_cast<driver_impl*>(this)->choose_ranking(context);
	}
	void driver::report_outcome(const char * uuid, const char * outcome_data)
	{
		return static_cast<driver_impl*>(this)->report_outcome(uuid, outcome_data);
	}
	void driver::report_outcome(const char * uuid, float reward)
	{
		return static_cast<driver_impl*>(this)->report_outcome(uuid, reward);
	}
}
