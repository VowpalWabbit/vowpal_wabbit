/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
*/

#include "ds_explore.h"

#include <math.h>

namespace Microsoft {
  namespace DecisionService {
    using namespace std;
    
    SoftmaxExplorer::SoftmaxExplorer(float lambda, float min_epsilon)
        : _lambda(lambda), _min_epsilon(min_epsilon)
    { }

    // from cb_explore_adf.cc 
    ActionProbabilities SoftmaxExplorer::explore(PredictorContainer& container)
    {
        DecisionServicePrediction& prediction = *container.begin();

        float norm = 0.;
        float max_score = prediction.max_score();

        // copy scores
        ActionProbabilities probability_distribution(prediction.num_actions()); 
    
        // initialize action/probabilities 
        vector<float>& scores = prediction.scores();
        for(size_t i=0;i<probability_distribution.size();++i)
        {
            float prob = exp(_lambda*(scores[i] - max_score));
            norm += prob;
             
            probability_distribution[i].probability = prob;
        }

        // normalize
        for (ActionProbability& actionProb : probability_distribution)
            actionProb.probability /= norm;

        probability_distribution.safety(_min_epsilon, true);
        probability_distribution.sort_by_probabilities_desc();

        return probability_distribution;
    }    
  }
}