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

		int driver_impl::ranking_request(const char * uuid, const char * context, ranking_response & response, api_status * status)
		{
			PRECONDITIONS(is_null_or_empty(uuid, status), return DS_INVALID_ARGUMENT);
			PRECONDITIONS(is_null_or_empty(context, status), return DS_INVALID_ARGUMENT);

			/* DO SOMETHING */
			//GET SCORES FOR TEST
			std::vector<std::pair<int, float>> ranking;
			ranking.push_back(std::pair<int, float>(2, 0.4f));
			ranking.push_back(std::pair<int, float>(1, 0.3f));
			ranking.push_back(std::pair<int, float>(4, 0.2f));
			ranking.push_back(std::pair<int, float>(3, 0.1f));
			std::string model_id = "model_id";

			//send the serialized event to the backend
			ranking_event evt(uuid, context, ranking, model_id);
			_logger.append_ranking(evt.serialize());

			//set the response
			response.set_uuid(uuid);
			response.set_ranking(ranking);

			return DS_SUCCESS;
		}

		//here the uuid is auto-generated
		int driver_impl::ranking_request(const char * context, ranking_response & response, api_status * status)
		{
			return ranking_request(context, boost::uuids::to_string(boost::uuids::random_generator()()).c_str(), response, status);
		}

		int report_outcome(const char* uuid, const char* outcome_data, api_status * status)
		{
			PRECONDITIONS(is_null_or_empty(uuid, status), return DS_INVALID_ARGUMENT);
			PRECONDITIONS(is_null_or_empty(outcome_data, status), return DS_INVALID_ARGUMENT);

			//send the serialized event to the backend
			outcome_event evt(uuid, outcome_data);
			_logger.append_outcome(evt.serialize());

			return DS_SUCCESS;
		}

		int report_outcome(const char* uuid, float reward, api_status * status)
		{
			return report_outcome(uuid, std::to_string(reward).c_str(), status);
		}

		driver_impl(const utility::config_collection& config)
			: _configuration(config),
			_logger(config)
		{}
	};

	//driver implementation
	driver::~driver() {}

	driver::driver(const utility::config_collection& config)
	{
		_pimpl = std::unique_ptr<driver_impl>(new driver_impl(config));
	}
	int driver::ranking_request(const char * uuid, const char * context_json, ranking_response & response, api_status * status)
	{
		return _pimpl->ranking_request(uuid, context_json, response, status);
	}
	int driver::ranking_request(const char * context_json, ranking_response & response, api_status * status)
	{
		return _pimpl->ranking_request(context_json, response, status);
	}
	int driver::report_outcome(const char * uuid, const char * outcome_data, api_status * status)
	{
		return _pimpl->report_outcome(uuid, outcome_data, status);
	}
	int driver::report_outcome(const char * uuid, float reward, api_status * status)
	{
		return _pimpl->report_outcome(uuid, reward, status);
	}
}
