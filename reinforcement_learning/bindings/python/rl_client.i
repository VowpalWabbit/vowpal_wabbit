%module(directors="1") rl_client

%{
#define SWIG_FILE_WITH_INIT
#include "py_api.h"
#include "../../include/constants.h"
#include "../../include/err_constants.h"
#include "config_collection.h"
%}

%include <exception.i>
%include <std_string.i>
%include <std_vector.i>

%template(vectori) std::vector<int>;
%template(vectorf) std::vector<float>;

%feature("director") reinforcement_learning::python::error_callback;

%exception {
  try {
    $action
  } catch(const std::runtime_error& e) {
    SWIG_exception(SWIG_RuntimeError, const_cast<char*>(e.what()));
  }
}

%include "py_api.h"
%include "../../include/constants.h"
%include "../../include/err_constants.h"
%include "../../include/config_collection.h"

%feature("pythonprepend") reinforcement_learning::python::live_model::live_model %{
    # If the overloaded version with a callback is called stash a reference to it so that the object is not deleted prematurely.
    if len(args) == 2:
        self.callback = args[1]
%}

namespace reinforcement_learning {
  namespace python {

    class live_model {
    public:
      live_model(const reinforcement_learning::utility::config_collection config, error_callback& callback);
      live_model(const reinforcement_learning::utility::config_collection config);

      void init();

      %rename(choose_rank_impl) choose_rank;
      reinforcement_learning::python::ranking_response choose_rank(const char* uuid, const char* context_json);
      // Uuid is auto-generated.
      reinforcement_learning::python::ranking_response choose_rank(const char* context_json);

      void report_outcome(const char* uuid, const char* outcome_data);
      void report_outcome(const char* uuid, float reward);

      %pythoncode %{
        def choose_rank(self, *args):
            ranking_response = self.choose_rank_impl(*args)
            if len(args) == 1:
                return ranking_response.model_id, ranking_response.chosen_action_id, list(zip(ranking_response.action_ids, ranking_response.probabilities)), ranking_response.uuid
            return ranking_response.model_id, ranking_response.chosen_action_id, list(zip(ranking_response.action_ids, ranking_response.probabilities))
      %}
    };
  }
}
