
#include "ds_explore.h"
#include <algorithm>

namespace Microsoft {
  namespace DecisionService {
    using namespace std;

    EpsilonGreedyExplorer::EpsilonGreedyExplorer(float epsilon)
      : _epsilon(epsilon)
    { }

    struct ScoreComparator
    {
      const vector<float>& _scores;

      ScoreComparator(const vector<float>& scores)
        : _scores(scores) 
      { }

      bool operator()(ActionProbability a, ActionProbability b)
      {
        return _scores[a.action] < _scores[b.action];
      }
    };

    ActionProbabilities EpsilonGreedyExplorer::explore(PredictorContainer& container)
    {
      DecisionServicePrediction& prediction = *container.begin();

      float prob = _epsilon/(float)prediction.num_actions();

      // size & initialize vector to prob 
      ActionProbabilities probability_distribution(prediction.num_actions(), prob);

      // boost the top element 
      probability_distribution[prediction.top_action()].probability += 1.f - _epsilon;

      // we also need to propagate the order produced by the scores
      sort(probability_distribution.begin(), probability_distribution.end(), ScoreComparator(prediction.scores()));

      return probability_distribution;
    }
  }
}
