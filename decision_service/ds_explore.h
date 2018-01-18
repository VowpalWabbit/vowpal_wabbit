#pragma once

#ifndef SWIG
#include "ds_predictor_container.h"
#endif

#include <vector>
//#include <cstdint>

#ifndef DISABLE_NAMESPACE
namespace Microsoft {
  namespace DecisionService {
#endif      
      #ifndef SWIG
      struct ActionProbability
      {
          int action;

          float probability;
      };

      class ActionProbabilities : public std::vector<ActionProbability>
      {
      public:
        ActionProbabilities(size_t count);

        ActionProbabilities(size_t count, float initial_probability);

        void safety(float min_prob, bool zeros);

        size_t sample(float draw);

        void swap_first(size_t index);

        void sort_by_probabilities_desc();

        // probabilities ordered by action
        vector<float> probabilities();

        // actions as currently ordered
        vector<int> actions();
      };

      #endif

      class IExplorer
      {
          public:
            virtual ~IExplorer();

// TODO: excluding from swig interface now to avoid PredictorContainer leakage and questions on memory for std::vector<float>&
//       assuming the exploration algos are always
#ifndef SWIG
            // returns distribution over actions... 
            virtual ActionProbabilities explore(PredictorContainer& container) = 0;
#endif
      };

      class EpsilonGreedyExplorer : public IExplorer
      {
          float _epsilon;

          public:
            EpsilonGreedyExplorer(float epsilon);

#ifndef SWIG
            virtual ActionProbabilities explore(PredictorContainer& container);
#endif
      };

      class SoftmaxExplorer : public IExplorer
      {
          float _lambda;
          float _min_epsilon;

          public:
            SoftmaxExplorer(float lambda, float min_epsilon = 0);

#ifndef SWIG
            virtual ActionProbabilities explore(PredictorContainer& container);
#endif
      };

      class BagExplorer : public IExplorer
      {
          float _min_epsilon;

          public:
            BagExplorer(float min_epsilon = 0);

#ifndef SWIG
            virtual ActionProbabilities explore(PredictorContainer& container);
#endif
      };

/*
      class CoverExplorer : public IExplorer
      {
          public:
            virtual const std::vector<ActionProbability> explore(PredictorContainer& container);
      };
      */
  }
}