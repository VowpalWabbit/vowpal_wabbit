#include "ds_explore.h"

namespace Microsoft {
  namespace DecisionService {
    using namespace std;

    CoverExplorer::CoverExplorer(float min_epsilon)
        : _min_epsilon(min_epsilon)
    { }

    const std::vector<float>& CoverExplorer::explore(PredictorContainer& container)
    {
        // TODO: unclear what to do here... need to learn accross?
        size_t counter = 1;
        auto it = container.begin();
        size_t num_actions = it->num_actions();

        float additive_probability = 1.f / (float)container.size();
        float min_prob = min(1.f / num_actions, 1.f / (float)sqrt(counter * num_actions));

        _probability_distribution.resize(num_actions);

        // TODO: unclear why += ? 
        _probability_distribution[it->top_action()] += additive_probability;

        float norm = min_prob * num_actions + (additive_probability - min_prob);
        for (++it;it != container.end();++it)
        {
            size_t action = it->top_action();

            /// 
            uint32_t action = preds[0].action;
            if (probs[action].score < min_prob)
                norm += max(0, additive_probability - (min_prob - probs[action].score));
            else
                norm += additive_probability;
            probs[action].score += additive_probability;
        }

        CB_EXPLORE::safety(data.action_probs, min_prob * num_actions, !data.nounif);

        qsort((void*) probs.begin(), probs.size(), sizeof(action_score), reverse_order);
        for (size_t i = 0; i < num_actions; i++)
            preds[i] = probs[i];

        ++data.counter;




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
  