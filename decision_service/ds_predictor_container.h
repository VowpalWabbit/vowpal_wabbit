/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
*/

#pragma once

#include <vector>

namespace Microsoft {
  namespace DecisionService {
 
    class PredictorContainer
      {
        DecisionServicePredictors* _predictors;
        
        // cache _prediction results
        DecisionServicePrediction* _predictions;

        std::vector<int> _previous_decisions;

        DecisionServicePrediction* prediction(size_t index);

      public:
        PredictorContainer(DecisionServicePredictors* predictors);

        PredictorContainer(DecisionServicePredictors* predictors, const std::vector<int>& previous_decisions);
        
        // move ctor
        PredictorContainer(PredictorContainer&& other);

        // don't allow copy
        PredictorContainer(const PredictorContainer& that) = delete;

        ~PredictorContainer();

        iterator begin();

        iterator end();

        size_t count();

        class iterator
        {
            PredictorContainer* _container;

            size_t _index;
        
        public:
            iterator(PredictorContainer* container, int index);

            DecisionServicePrediction& operator*();

            DecisionServicePrediction* operator->() const;

            // pre-increment
            iterator& operator++(); 

            // post-increment
            iterator operator++(int); 
        };

        friend class iterator;
      };

  }
}