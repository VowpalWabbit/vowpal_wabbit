#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/random_generator.hpp>

#include "utility/context_helper.h"
#include "sender.h"
#include "api_status.h"
#include "configuration.h"
#include "error_callback_fn.h"
#include "ranking_response.h"
#include "live_model_impl.h"
#include "ranking_event.h"
#include "err_constants.h"
#include "constants.h"
#include "vw_model/safe_vw.h"
#include "trace_logger.h"
#include "explore_internal.h"
#include "hash.h"
#include "factory_resolver.h"

// Some namespace changes for more concise code
namespace e = exploration;
using namespace std;

namespace reinforcement_learning {
  // Some namespace changes for more concise code
  namespace m = model_management;
  namespace u = utility;

  // Some typdefs for more concise code
  using vw_ptr = std::shared_ptr<safe_vw>;
  using pooled_vw = utility::pooled_object_guard<safe_vw, safe_vw_factory>;

  int check_null_or_empty(const char* arg1, const char* arg2, api_status* status);
  int check_null_or_empty(const char* arg1, api_status* status);

  void default_error_callback(const api_status& status, void* watchdog_context) {
    auto watchdog = static_cast<utility::watchdog*>(watchdog_context);
    watchdog->set_unhandled_background_error(true);
  }

  int live_model_impl::init(api_status* status) {
    RETURN_IF_FAIL(init_trace(status));
    RETURN_IF_FAIL(init_model(status));
    RETURN_IF_FAIL(init_model_mgmt(status));
    RETURN_IF_FAIL(init_loggers(status));
    _initial_epsilon = _configuration.get_float(name::INITIAL_EPSILON, 0.2f);
    const char* app_id = _configuration.get(name::APP_ID, "");
    _seed_shift = uniform_hash(app_id, strlen(app_id), 0);
    return error_code::success;
  }

  int live_model_impl::choose_rank(const char* event_id, const char* context, unsigned int flags, ranking_response& response,
    api_status* status) {
    response.clear();
    //clear previous errors if any
    api_status::try_clear(status);

    //check arguments
    RETURN_IF_FAIL(check_null_or_empty(event_id, context, status));
    if (!_model_data_received) {
      RETURN_IF_FAIL(explore_only(event_id, context, response, status));
      response.set_model_id("N/A");
    }
    else {
      RETURN_IF_FAIL(explore_exploit(event_id, context, response, status));
    }
    response.set_event_id(event_id);
    RETURN_IF_FAIL(_ranking_logger->log(event_id, context, flags, response, status));

    // Check watchdog for any background errors. Do this at the end of function so that the work is still done.
    if (_watchdog.has_background_error_been_reported()) {
      RETURN_ERROR_LS(_trace_logger.get(), status, unhandled_background_error_occurred);
    }

    return error_code::success;
  }

  //here the event_id is auto-generated
  int live_model_impl::choose_rank(const char* context, unsigned int flags, ranking_response& response, api_status* status) {
    return choose_rank(boost::uuids::to_string(boost::uuids::random_generator()()).c_str(), context, flags, response,
      status);
  }

  int live_model_impl::report_action_taken(const char* event_id, api_status* status) {
    // Clear previous errors if any
    api_status::try_clear(status);
    // Send the outcome event to the backend
    return _outcome_logger->report_action_taken(event_id, status);
  }

  int live_model_impl::report_outcome(const char* event_id, const char* outcome, api_status* status) {
    // Check arguments
    RETURN_IF_FAIL(check_null_or_empty(event_id, outcome, status));
    return report_outcome_internal(event_id, outcome, status);
  }

  int live_model_impl::report_outcome(const char* event_id, float outcome, api_status* status) {
    // Check arguments
    RETURN_IF_FAIL(check_null_or_empty(event_id, status));
    return report_outcome_internal(event_id, outcome, status);
  }

  live_model_impl::live_model_impl(
    const utility::configuration& config,
    const error_fn fn,
    void* err_context,
    trace_logger_factory_t* trace_factory,
    data_transport_factory_t* t_factory,
    model_factory_t* m_factory,
    sender_factory_t* sender_factory
  )
    : _configuration(config),
      _error_cb(fn, err_context),
      _data_cb(_handle_model_update, this),
      _watchdog(&_error_cb),
      _trace_factory(trace_factory),
      _t_factory{t_factory},
      _m_factory{m_factory},
      _sender_factory{sender_factory} {
    // If there is no user supplied error callback, supply a default one that does nothing but report unhandled background errors.
    if (fn == nullptr) {
      _error_cb.set(&default_error_callback, &_watchdog);
    }

    if (_configuration.get_bool(name::MODEL_BACKGROUND_REFRESH, value::MODEL_BACKGROUND_REFRESH)) {
      _bg_model_proc = std::make_unique<utility::periodic_background_proc<model_management::model_downloader>>(config.get_int(name::MODEL_REFRESH_INTERVAL_MS, 60 * 1000), _watchdog, "Model downloader", &_error_cb);
    }
  }

  int live_model_impl::init_trace(api_status* status) {
    const auto trace_impl = _configuration.get(name::TRACE_LOG_IMPLEMENTATION, value::NULL_TRACE_LOGGER);
    i_trace* plogger;
    RETURN_IF_FAIL(_trace_factory->create(&plogger, trace_impl,_configuration, nullptr, status));
    _trace_logger.reset(plogger);
    TRACE_INFO(_trace_logger, "API Tracing initialized");
    _watchdog.set_trace_log(_trace_logger.get());
    return error_code::success;
  }

  int live_model_impl::init_model(api_status* status) {
    const auto model_impl = _configuration.get(name::MODEL_IMPLEMENTATION, value::VW);
    m::i_model* pmodel;
    RETURN_IF_FAIL(_m_factory->create(&pmodel, model_impl, _configuration, _trace_logger.get(), status));
    _model.reset(pmodel);
    return error_code::success;
  }

  int live_model_impl::init_loggers(api_status* status) {
    const auto ranking_sender_impl = _configuration.get(name::INTERACTION_SENDER_IMPLEMENTATION, value::INTERACTION_EH_SENDER);
    i_sender* ranking_sender;
    RETURN_IF_FAIL(_sender_factory->create(&ranking_sender, ranking_sender_impl, _configuration, &_error_cb, _trace_logger.get(), status));
    _ranking_logger.reset(new interaction_logger(_configuration, ranking_sender, _watchdog, &_error_cb));
    RETURN_IF_FAIL(_ranking_logger->init(status));

    const auto outcome_sender_impl = _configuration.get(name::OBSERVATION_SENDER_IMPLEMENTATION, value::OBSERVATION_EH_SENDER);
    i_sender* outcome_sender;
    RETURN_IF_FAIL(_sender_factory->create(&outcome_sender, outcome_sender_impl, _configuration, &_error_cb, _trace_logger.get(), status));
    _outcome_logger.reset(new observation_logger(_configuration, outcome_sender, _watchdog, &_error_cb));
    RETURN_IF_FAIL(_outcome_logger->init(status));

    return error_code::success;
  }

  void inline live_model_impl::_handle_model_update(const m::model_data& data, live_model_impl* ctxt) {
    ctxt->handle_model_update(data);
  }

  void live_model_impl::handle_model_update(const model_management::model_data& data) {
    api_status status;
    if (_model->update(data, &status) != error_code::success) {
      _error_cb.report_error(status);
      return;
    }
    _model_data_received = true;
  }

  int live_model_impl::explore_only(const char* event_id, const char* context, ranking_response& response,
    api_status* status) const {

    // Generate egreedy pdf
    size_t action_count = 0;
    RETURN_IF_FAIL(utility::get_action_count(action_count, context, _trace_logger.get(), status));

    vector<float> pdf(action_count);
    // Generate a pdf with epsilon distributed between all action.
    // The top action gets the remaining (1 - epsilon)
    // Assume that the user's top choice for action is at index 0
    const auto top_action_id = 0;
    auto scode = e::generate_epsilon_greedy(_initial_epsilon, top_action_id, begin(pdf), end(pdf));
    if (S_EXPLORATION_OK != scode) {
      RETURN_ERROR_LS(_trace_logger.get(), status, exploration_error) << "Exploration error code: " << scode;
    }

    // The seed used is composed of uniform_hash(app_id) + uniform_hash(event_id)
    const uint64_t seed = uniform_hash(event_id, strlen(event_id), 0) + _seed_shift;

    // Pick a slot using the pdf. NOTE: sample_after_normalizing() can change the pdf
    uint32_t chosen_index;
    scode = e::sample_after_normalizing(seed, begin(pdf), end(pdf), chosen_index);

    if (S_EXPLORATION_OK != scode) {
      RETURN_ERROR_LS(_trace_logger.get(), status, exploration_error) << "Exploration error code: " << scode;
    }

    // NOTE: When there is no model, the rank
    // step was done by the user.  i.e. Actions are already in ranked order
    // If there were an action list it would be [0,1,2,3,4..].  The index
    // of the list matches the action_id.  There is no need to generate this
    // list of actions we can use the index into this list as a proxy for the
    // actual action_id.
    // i.e  chosen_index == action[chosen_index]
    // Why is this documented?  Because explore_exploit uses a model and we
    // cannot make the same assumption there.  (Bug was fixed)

    // Setup response with pdf from prediction and chosen action
    // Chosen action goes first.  First action gets swapped with chosen action
    response.push_back(chosen_index, pdf[chosen_index]);
    for (size_t idx = 1; idx < pdf.size(); ++idx) {
      const auto cur_idx = chosen_index != idx ? idx : 0;
      response.push_back(cur_idx, pdf[cur_idx]);
    }
    response.set_chosen_action_id(chosen_index);
    return error_code::success;
  }

  int live_model_impl::explore_exploit(const char* event_id, const char* context, ranking_response& response,
    api_status* status) const {
    // The seed used is composed of uniform_hash(app_id) + uniform_hash(event_id)
    const uint64_t seed = uniform_hash(event_id, strlen(event_id), 0) + _seed_shift;
    return _model->choose_rank(seed, context, response, status);
  }

  int live_model_impl::init_model_mgmt(api_status* status) {
    // Initialize transport for the model using transport factory
    const auto tranport_impl = _configuration.get(name::MODEL_SRC, value::AZURE_STORAGE_BLOB);
    m::i_data_transport* ptransport;
    RETURN_IF_FAIL(_t_factory->create(&ptransport, tranport_impl, _configuration, status));
    // This class manages lifetime of transport
    _transport.reset(ptransport);

    if (_bg_model_proc) {
      // Initialize background process and start downloading models
      _model_download = std::make_unique<m::model_downloader>(ptransport, &_data_cb, _trace_logger.get());
      return _bg_model_proc->init(_model_download.get(), status);
    }
    else {
      // update the model synchronously 
      model_management::model_data md;
      RETURN_IF_FAIL(_transport->get_data(md, status));
      RETURN_IF_FAIL(_model->update(md, status));
    }

    return error_code::success;
  }

  //helper: check if at least one of the arguments is null or empty
  int check_null_or_empty(const char* arg1, const char* arg2, api_status* status) {
    if (!arg1 || !arg2 || strlen(arg1) == 0 || strlen(arg2) == 0) {
      api_status::try_update(status, error_code::invalid_argument,
        "one of the arguments passed to the ds is null or empty");
      return error_code::invalid_argument;
    }
    return error_code::success;
  }

  int check_null_or_empty(const char* arg1, api_status* status) {
    if (!arg1 || strlen(arg1) == 0) {
      api_status::try_update(status, error_code::invalid_argument,
        "one of the arguments passed to the ds is null or empty");
      return error_code::invalid_argument;
    }
    return error_code::success;
  }

}
