#include "ds_api_status.h"
#include "ds_config_collection.h"
#include "ds_error_callback_fn.h"
#include "logger/ds_logger.h"
#include "ds_ranking_response.h"
#include "ds_driver_impl.h"
#include "ds_event.h"
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/random_generator.hpp>

// this macro assumes that success_code equals 0
#define TRY_OR_RETURN(x) do { \
  int retval = (x); \
  if (retval != 0) { \
    return retval; \
  } \
} while (0)
// Why while(0) ? It make the macro safe under various conditions. Check link below
// https://stackoverflow.com/questions/257418/do-while-0-what-is-it-good-for

namespace decision_service
{
  int check_null_or_empty(const char* arg1, const char* arg2, api_status* status);

  int driver_impl::ranking_request(const char* uuid, const char* context, ranking_response& response, api_status* status) {
      //clear previous errors if any
      api_status::try_clear(status);

      //check arguments
      TRY_OR_RETURN(check_null_or_empty(uuid, context, status));

      /* GET ACTIONS PROBABILITIES FROM VW */

      /**/ // TODO: replace with call to parse example and predict
           /**/ // TODO: once that is complete
      std::vector<std::pair<int, float>> action_proba;
      std::string model_id = "model_id";
      action_proba.push_back(std::pair<int, float>(2, 0.4f));
      action_proba.push_back(std::pair<int, float>(1, 0.3f));
      action_proba.push_back(std::pair<int, float>(4, 0.2f));
      action_proba.push_back(std::pair<int, float>(3, 0.1f));

      //send the ranking event to the backend
      ranking_event evt(uuid, context, action_proba, model_id);
      TRY_OR_RETURN(_logger.append_ranking(evt.serialize(), status));

      response.set_uuid(uuid);
      response.set_ranking(action_proba);

      return error_code::success;
  }

  //here the uuid is auto-generated
  int driver_impl::ranking_request(const char* context, ranking_response& response, api_status* status) {
    return ranking_request(context, boost::uuids::to_string(boost::uuids::random_generator()( )).c_str(), response,
      status);
  }

  int driver_impl::report_outcome(const char* uuid, const char* outcome_data, api_status* status) {
    //clear previous errors if any
    api_status::try_clear(status);

    //check arguments
    TRY_OR_RETURN(check_null_or_empty(uuid, outcome_data, status));

    //send the outcome event to the backend
    outcome_event evt(uuid, outcome_data);
    TRY_OR_RETURN(_logger.append_outcome(evt.serialize(), status));

    return error_code::success;
  }

  int driver_impl::report_outcome(const char* uuid, float reward, api_status* status) {
    return report_outcome(uuid, std::to_string(reward).c_str(), status);
  }

  driver_impl::driver_impl(const utility::config_collection& config, error_fn fn, void* err_context)
      : _configuration(config),
      _error_cb(fn, err_context),
      _logger(config, &_error_cb) {
  }
}