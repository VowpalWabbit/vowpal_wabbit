#include <boost/uuid/uuid_generators.hpp> // generators
#include <boost/uuid/uuid_io.hpp>         // streaming operators etc.
#include "ds_driver.h"
#include "logger/ds_logger.h"
#include "ds_driver_impl.h"

namespace decision_service
{
  //driver implementation
  driver::~driver()
  {
  }

  driver::driver(const utility::config_collection& config, error_fn fn, void* err_context)
  {
    _pimpl = std::unique_ptr<driver_impl>(new driver_impl(config, fn, err_context));
  }

  int driver::ranking_request(const char* uuid, const char* context_json, ranking_response& response,
                              api_status* status)
  {
    return _pimpl->ranking_request(uuid, context_json, response, status);
  }

  int driver::ranking_request(const char* context_json, ranking_response& response, api_status* status)
  {
    return _pimpl->ranking_request(context_json, response, status);
  }

  int driver::report_outcome(const char* uuid, const char* outcome_data, api_status* status)
  {
    return _pimpl->report_outcome(uuid, outcome_data, status);
  }

  int driver::report_outcome(const char* uuid, float reward, api_status* status)
  {
    return _pimpl->report_outcome(uuid, reward, status);
  }

  //helper: check if at least one of the arguments is null or empty
  int check_null_or_empty(const char* arg1, const char* arg2, api_status* status)
  {
    if (!arg1 || !arg2 || strlen(arg1) == 0 || strlen(arg2) == 0)
    {
      api_status::try_update(status, error_code::invalid_argument,
                             "one of the arguments passed to the ds is null or empty");
      return error_code::invalid_argument;
    }

    return error_code::success;
  }
}
