%module(directors="1") rlinference

%{
#define SWIG_FILE_WITH_INIT
#include "py_api.h"
#include "../include/constants.h"
#include "../include/err_constants.h"
#include "config_collection.h"
%}

%include <exception.i>
%include <std_string.i>
%include <std_pair.i>
%include <std_vector.i>

%template(vectori) std::vector<int>;
%template(vectorf) std::vector<float>;

%include "py_api.h"
%include "../include/constants.h"
%include "../include/err_constants.h"
%include "../include/config_collection.h"

namespace reinforcement_learning {
  namespace python {

  %feature("director") error_callback;
  class error_callback {
    public:
      virtual void on_error(int error_code, const std::string& error_message) {}
      virtual ~error_callback() = default;
    };


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
            return ranking_response.uuid, ranking_response.modelid, ranking_response.chosen_action_id, list(zip(ranking_response.action_ids, ranking_response.probabilities))
      %}

      private:
        reinforcement_learning::live_model impl;
    };
  }
}
