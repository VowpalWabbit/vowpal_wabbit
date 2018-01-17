#include "ds_explore.h"

#include <math.h>

namespace Microsoft {
  namespace DecisionService {
    SoftmaxExplorer::SoftmaxExplorer(float lambda, float min_epsilon)
        : _lambda(lambda), _min_epsilon(min_epsilon)
    { }

    // from cb_explore_adf.cc 
    const std::vector<float> SoftmaxExplorer::explore(PredictorContainer& container)
    {
        DecisionServicePrediction& prediction = *container.begin();

        float norm = 0.;
        float max_score = prediction.max_score();

        // copy scores
        std::vector<float> probability_distribution(prediction.scores()); 
    
        for (float& prob : probability_distribution)
        {
            prob = exp(_lambda*(prob - max_score));
            norm += prob;
        }

        // normalize
        for (float& prob : probability_distribution)
            prob /= norm;

        safety(probability_distribution, _min_epsilon, true);

        return probability_distribution;
    }    
  }
}