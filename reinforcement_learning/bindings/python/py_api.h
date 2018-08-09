
#pragma once

#include <vector>
#include <string>

#include "config_collection.h"
#include "live_model.h"

namespace reinforcement_learning {
  namespace python {

    reinforcement_learning::utility::config_collection create_config_from_json(const std::string& config_json);

    class error_callback {
    public:
      virtual void on_error(int error_code, const std::string& error_message) {}
      virtual ~error_callback() = default;
    };

    struct ranking_response {
      std::string uuid;
      std::string model_id;
      size_t chosen_action_id;
      std::vector<int> action_ids;
      std::vector<float> probabilities;
    };

// This is defined in the interface file instead so that custom Python can be injected
#ifndef SWIG
    class live_model {
    public:
      live_model(const reinforcement_learning::utility::config_collection config, error_callback& callback);
      live_model(const reinforcement_learning::utility::config_collection config);

      void init();

      ranking_response choose_rank(const char* uuid, const char* context_json);
      // Uuid is auto-generated.
      ranking_response choose_rank(const char* context_json);

      void report_outcome(const char* uuid, const char* outcome_data);
      void report_reward(const char* uuid, float reward);

      private:
        reinforcement_learning::live_model impl;
    };
#endif
  }
}
