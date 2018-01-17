#include "ds_explore.h"

namespace Microsoft {
  namespace DecisionService {
    using namespace std;

    EpsilonGreedyExplorer::EpsilonGreedyExplorer(float epsilon)
        : _epsilon(epsilon)
    { }

    const std::vector<float> EpsilonGreedyExplorer::explore(PredictorContainer& container)
    {
        DecisionServicePrediction& prediction = *container.begin();

        float prob = _epsilon/(float)prediction.num_actions();

        // size & initialize vector to prob 
        std::vector<float> probability_distribution(prediction.num_actions(), prob);

        // boost the top element 
        probability_distribution[prediction.top_action()] += 1.f - _epsilon;

        return probability_distribution;
    }
  }
}
