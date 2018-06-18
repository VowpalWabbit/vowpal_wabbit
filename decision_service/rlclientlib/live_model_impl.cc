#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/random_generator.hpp>

#include "api_status.h"
#include "config_collection.h"
#include "error_callback_fn.h"
#include "logger/logger.h"
#include "ranking_response.h"
#include "live_model_impl.h"
#include "ranking_event.h"
#include "err_constants.h"
#include "factory_resolver.h"
#include "constants.h"
#include "model_download.h"
#include "context_helper.h"
#include "vw_model/safe_vw.h"
#include "../../explore/explore_internal.h"

// Some namespace changes for more concise code
namespace e = exploration;
using namespace std;

namespace reinforcement_learning
{
  // Some namespace changes for more concise code
  namespace m = model_management;

  // Some typdefs for more concise code
  using vw_ptr = std::shared_ptr<safe_vw>;
  using pooled_vw = utility::pooled_object_guard<safe_vw, safe_vw_factory>;

  int check_null_or_empty(const char* arg1, const char* arg2, api_status* status);

  int live_model_impl::init(api_status* status) {
    int scode = _logger.init(status);
    TRY_OR_RETURN(scode);
    scode = init_model(status);
    TRY_OR_RETURN(scode);
    scode = init_model_mgmt(status);
    TRY_OR_RETURN(scode);
    _initial_epsilon = _configuration.get_float(name::INITIAL_EPSILON, 0.2f);
    return scode;
  }

  int live_model_impl::choose_rank(const char* uuid, const char* context, ranking_response& response, api_status* status) 
  {
    //clear previous errors if any
    api_status::try_clear(status);

    //check arguments
    TRY_OR_RETURN(check_null_or_empty(uuid, context, status));

    int scode;
    if(!_model_data_received) {
      scode = explore_only(uuid, context, response, status);
      TRY_OR_RETURN(scode);
      response.set_model_id("N/A");
    }
    else {
      scode = explore_exploit(uuid, context, response, status);
      TRY_OR_RETURN(scode);
    }

    response.set_uuid(uuid);

    // Serialize the event
    _buff.seekp(0, std::ostringstream::beg);
    ranking_event::serialize(_buff, uuid, context, response);
    auto sbuf = _buff.str();

    // Send the ranking event to the backend
    TRY_OR_RETURN(_logger.append_ranking(sbuf, status));

    return error_code::success;
  }

  //here the uuid is auto-generated
  int live_model_impl::choose_rank(const char* context, ranking_response& response, api_status* status) {
    return choose_rank(context, boost::uuids::to_string(boost::uuids::random_generator()( )).c_str(), response,
      status);
  }

  int live_model_impl::report_outcome(const char* uuid, const char* outcome_data, api_status* status) {
    // Clear previous errors if any
    api_status::try_clear(status);

    // Check arguments
    TRY_OR_RETURN(check_null_or_empty(uuid, outcome_data, status));

    // Serialize outcome
    _buff.seekp(0, _buff.beg);
    outcome_event::serialize(_buff,uuid, outcome_data);
    auto sbuf = _buff.str();

    // Send the outcome event to the backend
    TRY_OR_RETURN(_logger.append_outcome(sbuf, status));

    return error_code::success;
  }

  int live_model_impl::report_outcome(const char* uuid, float reward, api_status* status) {
    return report_outcome(uuid, to_string(reward).c_str(), status);
  }

  live_model_impl::live_model_impl(const utility::config_collection& config, const error_fn fn, void* err_context)
  : _configuration(config),
    _error_cb(fn, err_context), 
    _data_cb(_handle_model_update,this),
    _logger(config, &_error_cb),
    _transport(nullptr),
    _model(nullptr),
    _model_download(nullptr),
    _bg_model_proc(config.get_int(name::MODEL_REFRESH_INTERVAL, 60 * 5), &_error_cb)
  { }

int live_model_impl::init_model(api_status* status) {
  const auto model_impl = _configuration.get(name::MODEL_IMPLEMENTATION, value::VW);
  m::i_model* pmodel;
  TRY_OR_RETURN(model_factory.create(&pmodel, model_impl, _configuration,status));
  _model.reset(pmodel);
  return error_code::success;
}

void inline live_model_impl::_handle_model_update(const m::model_data& data, live_model_impl* ctxt) {
  ctxt->handle_model_update(data);
}

void live_model_impl::handle_model_update(const model_management::model_data& data) {
  api_status status;
  if(_model->update(data,&status) != error_code::success) {
    _error_cb.report_error(status);
    return;
  }
  _model_data_received = true;
}

int live_model_impl::explore_only(const char* uuid, const char* context,  ranking_response& response, api_status* status) const {
  
  // Generate egreedy pdf
  size_t action_count = 0;
  TRY_OR_RETURN(utility::get_action_count(action_count, context, status));
  vector<float> pdf(action_count);

  // Assume that the user's top choice for action is at index 0
  const auto top_action_id = 0;
  auto scode = e::generate_epsilon_greedy(_initial_epsilon, top_action_id, begin(pdf), end(pdf));
  if( S_EXPLORATION_OK != scode) {
    return report_error(status, error_code::exploration_error, "Exploration error code: ", scode);
  }

  // Pick using the pdf
  uint32_t choosen_action_id;
  scode = e::sample_after_normalizing(uuid, begin(pdf), end(pdf), choosen_action_id);
  if ( S_EXPLORATION_OK != scode ) {
    return report_error(status, error_code::exploration_error, "Exploration error code: ", scode);
  }

  // setup response
  for(size_t i = 0; i < pdf.size(); i++ ) {
    response.push_back(i, pdf[i]);
  }
  response.set_choosen_action_id(top_action_id);

  return error_code::success;
}

int live_model_impl::explore_exploit(const char* uuid, const char* context, ranking_response& response,
  api_status* status) const {
  return _model->choose_rank(uuid, context, response, status);
}

int live_model_impl::init_model_mgmt(api_status* status) {
  // Initialize transport for the model using transport factory
  const auto tranport_impl = _configuration.get(name::MODEL_SRC, value::AZURE_STORAGE_BLOB);
  m::i_data_transport* ptransport;
  TRY_OR_RETURN(data_transport_factory.create(&ptransport, tranport_impl, _configuration, status));
  // This class manages lifetime of transport
  this->_transport.reset(ptransport);

  // Initialize background process and start downloading models
  this->_model_download.reset(new m::model_download(ptransport, &_data_cb));
  return _bg_model_proc.init(_model_download.get(),status);
}
}
