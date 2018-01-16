#include "ds_explore.h"

namespace Microsoft {
  namespace DecisionService {
    using namespace std;

    BagExplorer::BagExplorer(float min_epsilon)
        : _min_epsilon(min_epsilon)
    { }

    const std::vector<float>& BagExplorer::explore(PredictorContainer& container)
    {
        // determine probability per model
        float prob = 1.f / (float)container.count();
        auto it = container.begin();
        
        _probability_distribution.resize(it->num_actions());

        // accumulate prob per top action of each model
        for(;it != end; ++it)
            _probability_distribution[it->top_action()] += prob;

        return safety(_min_epsilon, true);
    }    
  }
}
  