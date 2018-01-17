#pragma once

#ifndef SWIG
#include "ds_predictor_container.h"
#endif
#include <vector>

#ifndef DISABLE_NAMESPACE
namespace Microsoft {
  namespace DecisionService {
#endif      
      class IExplorer
      {
          protected:
            void safety(std::vector<float>& probability_distribution, float min_prob, bool zeros);

          public:
            virtual ~IExplorer();

// TODO: excluding from swig interface now to avoid PredictorContainer leakage and questions on memory for std::vector<float>&
//       assuming the exploration algos are always
#ifndef SWIG
            // returns distribution over actions... 
            virtual const std::vector<float> explore(PredictorContainer& container) = 0;
#endif
      };

      class EpsilonGreedyExplorer : public IExplorer
      {
          float _epsilon;

          public:
            EpsilonGreedyExplorer(float epsilon);

#ifndef SWIG
            virtual const std::vector<float> explore(PredictorContainer& container);
#endif
      };

      class SoftmaxExplorer : public IExplorer
      {
          float _lambda;
          float _min_epsilon;

          public:
            SoftmaxExplorer(float lambda, float min_epsilon = 0);

#ifndef SWIG
            virtual const std::vector<float> explore(PredictorContainer& container);
#endif
      };

      class BagExplorer : public IExplorer
      {
          float _min_epsilon;

          public:
            BagExplorer(float min_epsilon = 0);

#ifndef SWIG
            virtual const std::vector<float> explore(PredictorContainer& container);
#endif
      };

/*
      class CoverExplorer : public IExplorer
      {
          public:
            virtual const std::vector<float> explore(PredictorContainer& container);
      };
      */
  }
}