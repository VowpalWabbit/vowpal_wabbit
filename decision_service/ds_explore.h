#pragma once

#include "ds_api.h"

#include <vector>

namespace Microsoft {
  namespace DecisionService {
      
      class IExplorer
      {
          protected:
            std::vector<float> _probability_distribution;

            const std::vector<float>& safety(float min_prob, bool zeros);

          public:
            virtual ~IExporer() = 0;

            // returns distribution over actions... 
            virtual const std::vector<float>& explore(PredictorContainer& container) = 0;
      };

      class EpsilonGreedyExplorer : public IExplorer
      {
          float _epsilon;

          public:
            EpsilonGreedyExplorer(float epsilon);

            virtual const std::vector<float>& explore(PredictorContainer& container);
      }

      class SoftmaxExplorer : public IExplorer
      {
          float _lambda;
          float _min_epsilon;

          public:
            SoftmaxExplorer(float lambda, float min_epsilon = 0);

            virtual const std::vector<float>& explore(PredictorContainer& container);
      }

      class BagExplorer : public IExplorer
      {
          float _min_epsilon;

          public:
            BagExplorer(float min_epsilon = 0);

            virtual const std::vector<float>& explore(PredictorContainer& container);
      }

      class CoverExplorer : public IExplorer
      {
          public:
            virtual const std::vector<float>& explore(PredictorContainer& container);
      }
  }
}