/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
*/

#include "ds_predictors.h"

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <iostream>

namespace Microsoft {
  namespace DecisionService {
      using namespace std;

        void DecisionServicePrediction::set(const std::vector<float>& scores)
        {
            _scores = scores;
        }

        size_t DecisionServicePrediction::top_action() const
        {
            return std::max_element(_scores.begin(), _scores.end()) - _scores.begin();
        }

        float DecisionServicePrediction::max_score() const
        {
            return _scores[top_action()];
        }

        size_t DecisionServicePrediction::num_actions() const
        {
            return _scores.size();
        }

        std::vector<float>& DecisionServicePrediction::scores()
        {
            return _scores;
        }

      DecisionServicePredictors::DecisionServicePredictors() : DecisionServicePredictors(1)
      { }

      DecisionServicePredictors::DecisionServicePredictors(size_t count)
        : _count(count)
      { }
      
      DecisionServicePredictors::~DecisionServicePredictors()
      { }

      void DecisionServicePredictors::get_prediction_out(size_t index, const std::vector<int>& previous_decisions, DecisionServicePrediction* output_result) throw(std::exception)
      { 
        output_result->set(this->get_prediction(index, previous_decisions));
      }

      std::vector<float> DecisionServicePredictors::get_prediction(size_t index, const std::vector<int>& previous_decisions) throw(std::exception)
      {
          throw domain_error("Missing implemention: please provide std::vector<float> get_prediction(index, const std::vector<int>&)");
      }

      size_t DecisionServicePredictors::count() const
      {
          return _count;
      }
  }
}
  