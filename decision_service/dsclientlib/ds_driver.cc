#include "ds_driver.h"
#include "ds_event.h"
#include "logger/ds_logger.h"

#include <boost/uuid/uuid_generators.hpp> // generators
#include <boost/uuid/uuid_io.hpp>         // streaming operators etc.
#include "error_callback_fn.h"


//this macro assumes that success_code equals 0
#define TRY_OR_RETURN(x) do { \
  int retval = (x); \
  if (retval != 0) { \
    return retval; \
  } \
} while (0)

namespace decision_service {

	int check_null_or_empty(const char * arg1, const char * arg2, api_status* status);

	class driver_impl {
	public:
    	using error_fn = void(*)(const api_status&,void* user_context);

	private:
	  	utility::config_collection _configuration;
    	error_callback_fn _error_cb;
    	logger _logger;

	public:
		int ranking_request(const char * uuid, const char * context, ranking_response & response, api_status * status)
		{
	    	//clear previous errors if any
      		if (status != nullptr) status->clear();

			//check arguments
			TRY_OR_RETURN(check_null_or_empty(uuid, context, status));

			/* GET ACTIONS PROBABILITIES FROM VW */

			/**/ // TODO: replace with call to parse example and predict
			/**/ // TODO: once that is complete
				/**/ std::vector<std::pair<int, float>> action_proba;
			/**/ std::string model_id = "model_id";
			/**/ action_proba.push_back(std::pair<int, float>(2, 0.4f));
			/**/ action_proba.push_back(std::pair<int, float>(1, 0.3f));
			/**/ action_proba.push_back(std::pair<int, float>(4, 0.2f));
			/**/ action_proba.push_back(std::pair<int, float>(3, 0.1f));

			//send the ranking event to the backend
			ranking_event evt(uuid, context, action_proba, model_id);
			TRY_OR_RETURN(_logger.append_ranking(evt.serialize(), status));

			response.set_uuid(uuid);
			response.set_ranking(action_proba);

			return error_code::success;
		}

		//here the uuid is auto-generated
		int ranking_request(const char * context, ranking_response & response, api_status * status)
		{
			return ranking_request(context, boost::uuids::to_string(boost::uuids::random_generator()()).c_str(), response, status);
		}

		int report_outcome(const char* uuid, const char* outcome_data, api_status * status)
		{
	    	//clear previous errors if any
      		if (status != nullptr) status->clear();

			//check arguments
			TRY_OR_RETURN(check_null_or_empty(uuid, outcome_data, status));

			//send the outcome event to the backend
			outcome_event evt(uuid, outcome_data);
			TRY_OR_RETURN(_logger.append_outcome(evt.serialize(), status));

			return error_code::success;
		}

		int report_outcome(const char* uuid, float reward, api_status * status)
		{
			return report_outcome(uuid, std::to_string(reward).c_str(), status);
		}

		driver_impl(const utility::config_collection& config, error_fn fn = nullptr, void* err_context = nullptr)
			: _configuration(config),
        _error_cb(fn,err_context),
  			_logger(config,&_error_cb)
		{
		}

	};

	//driver implementation
	driver::~driver() {}

	driver::driver(const utility::config_collection& config, error_fn fn, void* err_context) 
	{
		_pimpl = std::unique_ptr<driver_impl>(new driver_impl(config,fn,err_context));
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

	//helper: check if at least one of the arguments is null or empty
	int check_null_or_empty(const char * arg1, const char * arg2, api_status* status)
	{
		if (!arg1 || !arg2 || strlen(arg1) == 0 || strlen(arg2) == 0) {
			api_status::try_update(status, error_code::invalid_argument, "one of the arguments passed to the ds is null or empty");
			return error_code::invalid_argument;
		}

		return error_code::success;
	}
}
