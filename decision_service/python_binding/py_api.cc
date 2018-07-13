#include "py_api.h"

#include "config_utility.h"
#include "config_collection.h"
#include "factory_resolver.h"

#include <Python.h>

namespace reinforcement_learning {
  namespace python {
    void dispatch_error_internal(const reinforcement_learning::api_status& status, error_callback* context) {
      // Obtain global interpreter lock to execute Python.
      PyGILState_STATE gstate;
      gstate = PyGILState_Ensure();

      // Callback defined by Python subclass.
      context->on_error(status.get_error_code(), status.get_error_msg());

      // Release the global interpreter lock.
      PyGILState_Release(gstate);
    }

    reinforcement_learning::utility::config_collection create_from_json(const std::string& config_json) {
      reinforcement_learning::utility::config_collection cc;
      reinforcement_learning::api_status as;
      auto result = reinforcement_learning::utility::config::create_from_json(config_json, cc, &as);

      return cc;
    }

    live_model::live_model(const reinforcement_learning::utility::config_collection config, error_callback& callback)
      : impl(config, &dispatch_error_internal, &callback, &data_transport_factory, &model_factory)
    {}

    live_model::live_model(const reinforcement_learning::utility::config_collection config)
      : impl(config, nullptr, nullptr, &data_transport_factory, &model_factory)
    {}

    void live_model::init() {
      impl.init();
    }

    ranking_response live_model::choose_rank(const char* uuid, const char* context_json) {
      reinforcement_learning::ranking_response rr;
      reinforcement_learning::api_status as;
      auto result = impl.choose_rank(uuid, context_json, rr, &as);
      // check api status and error code

      ranking_response py_rr;
      py_rr.uuid = rr.get_uuid();
      py_rr.modelid = rr.get_model_id();
      size_t chosen_action_id;
      result = rr.get_choosen_action_id(chosen_action_id, &as);
      // err check.
      py_rr.chosen_action_id = chosen_action_id;
      // Todo get actual values here
      py_rr.action_ids = {1,4};
      py_rr.probabilities = {3.4, 3.6};

      return py_rr;
    }
    // Uuid is auto-generated.
    ranking_response live_model::choose_rank(const char* context_json) {
      reinforcement_learning::ranking_response rr;
      reinforcement_learning::api_status as;
      auto result = impl.choose_rank(context_json, rr, &as);
      // check api status and error code

      ranking_response py_rr;
      py_rr.uuid = rr.get_uuid();
      py_rr.modelid = rr.get_model_id();
      size_t chosen_action_id;
      result = rr.get_choosen_action_id(chosen_action_id, &as);
      // err check.
      py_rr.chosen_action_id = chosen_action_id;
      // Todo get actual values here
      py_rr.action_ids = {1,4};
      py_rr.probabilities = {3.4, 3.6};

      return py_rr;
    }

    void live_model::report_outcome(const char* uuid, const char* outcome_data) {
      reinforcement_learning::api_status as;
      auto result = impl.report_outcome(uuid, outcome_data, &as);
      //check
    }

    void live_model::report_outcome(const char* uuid, float reward) {
      reinforcement_learning::api_status as;
      auto result = impl.report_outcome(uuid, reward, &as);
      //check
    }
  }
}