#include "ds_driver.h"
#include "ds_event.h"
#include "logger/ds_logger.h"

#include <boost/uuid/uuid_generators.hpp> // generators
#include <boost/uuid/uuid_io.hpp>         // streaming operators etc.

namespace decision_service {

	class driver_impl {
	  utility::config_collection _configuration;
		logger _logger;

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
			_logger.append_ranking(evt.serialize());

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
			_logger.append_outcome(evt.serialize());
		}

		void report_outcome(const char* uuid, float reward)
		{
			report_outcome(uuid, std::to_string(reward).c_str());
		}

		driver_impl(const utility::config_collection& config)
			: _configuration(config),
			_logger(config)
		{}
	};

	//driver implementation
  driver::~driver(){}

	driver::driver(const utility::config_collection& config)
	{
    _pimpl = std::unique_ptr<driver_impl>(new driver_impl(config));
	}
	ranking_response driver::choose_ranking(const char * uuid, const char * context)
	{
		return _pimpl->choose_ranking(uuid, context);
	}
	ranking_response driver::choose_ranking(const char * context)
	{
		return _pimpl->choose_ranking(context);
	}
	void driver::report_outcome(const char * uuid, const char * outcome_data)
	{
		return _pimpl->report_outcome(uuid, outcome_data);
	}
	void driver::report_outcome(const char * uuid, float reward)
	{
		return _pimpl->report_outcome(uuid, reward);
	}
}
