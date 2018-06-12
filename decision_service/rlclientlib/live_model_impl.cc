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
#include "bg_model_download.h"
#include "context_helper.h"
#include "../../explore/explore_internal.h"
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/random_generator.hpp>

using namespace std;
namespace e = exploration;

namespace reinforcement_learning
{
  namespace m = model_management;

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
    const string model_id = "N/A";
    if(!_model_data_received) {
      scode = explore_only(uuid, context, response, status);
      TRY_OR_RETURN(scode);
    }
    else {
      scode = explore_exploit(uuid, context, response, status);
      // TODO: POPULATE MODEL ID FROM VW
      // model_id = vw.model_id();
      TRY_OR_RETURN(scode);
    }

    response.set_uuid(uuid);

    // Serialize the event
    _buff.seekp(0, std::ostringstream::beg);
    ranking_event::serialize(_buff, uuid, context, response, model_id);
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

  live_model_impl::live_model_impl(const utility::config_collection& config, error_fn fn, void* err_context)
  : _model_data_received(false), _configuration(config),
    _error_cb(fn, err_context),
    _logger(config, &_error_cb) { }

int live_model_impl::init_model(api_status* status) {
  const auto model_impl = _configuration.get(name::MODEL, value::VW);
  m::i_model* pmodel;
  TRY_OR_RETURN(model_factory.create(&pmodel, model_impl, _configuration,status));
  _model.reset(pmodel);
  return error_code::success;
}

void inline live_model_impl::_handle_model_update(const m::model_data& data, live_model_impl* ctxt) {
  ctxt->handle_model_update(data);
}

void live_model_impl::handle_model_update(const model_management::model_data& data) {
  _model->update(data);
  _model_data_received = true;
}

int live_model_impl::explore_only(const char* uuid, const char* context,  ranking_response& response, api_status* status) const {
  
  // Generate egreedy pdf
  size_t action_count = 0;
  TRY_OR_RETURN(utility::get_action_count(action_count, context, status));
  vector<float> pdf(action_count);

  // assume that the user's top choice for action is at index 0
  const auto top_action_id = 0;
  auto scode = e::generate_epsilon_greedy(_initial_epsilon, top_action_id, begin(pdf), end(pdf));
  if( S_EXPLORATION_OK != scode) {
    return report_error(status, error_code::exploration_error, "Exploration error code: ", scode);
  }

  // pick usig the pdf
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
  api_status* status) {
  throw "not implemented";
}

int live_model_impl::init_model_mgmt(api_status* status) {
  const auto tranport_impl = _configuration.get(name::MODEL_SRC, value::AZURE_STORAGE_BLOB);
  m::i_data_transport* ptransport;
  TRY_OR_RETURN(data_transport_factory.create(&ptransport, tranport_impl, _configuration, status));
  _model_transport.reset(ptransport);
  _data_cb.reset(new m::data_callback_fn(_handle_model_update, this));
  _bg_model_download.reset(new m::bg_model_download( _model_transport.get(),_data_cb.get()));
  return error_code::success;
}
}
