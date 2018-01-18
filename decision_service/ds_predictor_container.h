/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
*/

#pragma once

#include <vector>
#include <cstddef>

#include "ds_predictors.h"

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

            friend bool operator==(const PredictorContainer::iterator& lhs, const  PredictorContainer::iterator& rhs);
            friend bool operator!=(const PredictorContainer::iterator& lhs, const  PredictorContainer::iterator& rhs);
        };

        iterator begin();

        iterator end();

        friend class iterator;
      };

      inline bool operator==(const PredictorContainer::iterator& lhs, const  PredictorContainer::iterator& rhs)
      {
        // play safe
        return lhs._index == rhs._index && lhs._container == rhs._container;
      }

      inline bool operator!=(const PredictorContainer::iterator& lhs, const  PredictorContainer::iterator& rhs){ return !(lhs == rhs); }
  }
}