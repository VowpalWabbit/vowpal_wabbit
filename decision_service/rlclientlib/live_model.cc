#include <boost/uuid/uuid_generators.hpp> // generators
#include <boost/uuid/uuid_io.hpp>         // streaming operators etc.
#include "live_model.h"
#include "logger/logger.h"
#include "live_model_impl.h"
#include "err_constants.h"

#define INIT_CHECK() do {                                       \
  if(!_initialized) {                                           \
    api_status::try_update(status, error_code::not_initialized, \
                "Library not initialized. Call init() first."); \
    return error_code::not_initialized;                         \
  }                                                             \
} while(0);                                                     \

namespace reinforcement_learning
{
  //live_model implementation
  live_model::~live_model()
  {
    delete _pimpl;
  }

  int live_model::init(api_status* status) {
    if ( _initialized )
      return error_code::success;

    const auto err_code = _pimpl->init(status);
    if ( err_code == error_code::success ) {
      _initialized = true;
    }

    return err_code;
  }

  live_model::live_model(const utility::config_collection& config, const error_fn fn, void* err_context) 
    : _pimpl(new live_model_impl(config, fn, err_context)),
    _initialized(false)
  {
  }

  int live_model::choose_rank(const char* uuid, const char* context_json, ranking_response& response,
                              api_status* status)
  {
    INIT_CHECK();
    return _pimpl->choose_rank(uuid, context_json, response, status);
  }

  int live_model::choose_rank(const char* context_json, ranking_response& response, api_status* status)
  {
    INIT_CHECK();
    return _pimpl->choose_rank(context_json, response, status);
  }

  int live_model::report_outcome(const char* uuid, const char* outcome_data, api_status* status)
  {
    INIT_CHECK();
    return _pimpl->report_outcome(uuid, outcome_data, status);
  }

  int live_model::report_outcome(const char* uuid, float reward, api_status* status)
  {
    INIT_CHECK();
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
