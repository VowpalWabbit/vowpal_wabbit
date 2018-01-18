#include "ds_explore.h"

namespace Microsoft {
  namespace DecisionService {
    using namespace std;

    BagExplorer::BagExplorer(float min_epsilon)
        : _min_epsilon(min_epsilon)
    { }

    ActionProbabilities BagExplorer::explore(PredictorContainer& container)
    {
        // determine probability per model
        float prob = 1.f / (float)container.count();
        auto it = container.begin();
        auto end = container.end();

        ActionProbabilities probability_distribution(it->num_actions());

        // accumulate prob per top action of each model
        for(;it != end; ++it)
            probability_distribution[it->top_action()].probability += prob;

        probability_distribution.safety(_min_epsilon, true);
        probability_distribution.sort_by_probabilities_desc();

        return probability_distribution;
    }    
  }
}
  