#include "ds_explore.h"

namespace Microsoft {
  namespace DecisionService {
    using namespace std;

    EpsilonGreedyExplorer::EpsilonGreedyExplorer(float epsilon)
        : _epsilon(epsilon)
    { }

    const std::vector<float>& EpsilonGreedyExplorer::explore(PredictorContainer& container)
    {
        DecisionServicePrediction& prediction = *container.begin();

        float prob = _epsilon/(float)prediction.num_actions();

        // size & initialize vector to prob 
        _probability_distribution.resize(prediction.num_actions(), prob);

        // boost the top element 
        _probability_distribution[prediction.top_action()] += 1.f - _epsilon;

        return _probability_distribution;
    }
  }
}
