#include "ds_explore.h"

#include <algorithm>
#include <stdexcept>

namespace Microsoft {
  namespace DecisionService {
    using namespace std;

    ActionProbabilities::ActionProbabilities(size_t count)
        : ActionProbabilities(count, 0.f)
    { }

    ActionProbabilities::ActionProbabilities(size_t count, float initial_probability)
        : vector(count, { 0, initial_probability })
    { 
        // initialize it 0...n
        int action = 0;
        for (ActionProbability& actionProb : *this)
            actionProb.action = action++;
    }

    void ActionProbabilities::safety(float min_prob, bool zeros)
    {
        //input: a probability distribution
        //output: a probability distribution with all events having probability > min_prob.  This includes events with probability 0 if zeros = true
        if (min_prob > 0.999) // uniform exploration
        {
            size_t support_size = size();
            if (!zeros)
            {
                // support_size -= count_if(probability_distribution.begin(), probability_distribution.end(), 
                //     [](ActionProbability actionProb) { return actionProb.probability == 0; });
  
                for (ActionProbability& actionProb : *this)
                    if (actionProb.probability == 0)
                        support_size--;
            }
        
            for (ActionProbability& actionProb : *this)
                if (zeros || actionProb.probability > 0)
                    actionProb.probability = 1.f / support_size;

            return;
        }

        min_prob /= size();
        float touched_mass = 0.;
        float untouched_mass = 0.;
        
        for (ActionProbability& actionProb : *this)
        {
            float& prob = actionProb.probability;
            if ((prob > 0 || (prob == 0 && zeros)) && prob <= min_prob)
            {
                touched_mass += min_prob;
                prob = min_prob;
            }
            else
                untouched_mass += prob;
        }   

        if (touched_mass > 0.)
        {
            if (touched_mass > 0.999)
                throw invalid_argument("Cannot safety this distribution");

            float ratio = (1.f - touched_mass) / untouched_mass;
            for (ActionProbability& actionProb : *this)
                if (actionProb.probability > min_prob)
                    actionProb.probability *= ratio;
        }
    }


    size_t ActionProbabilities::sample(float draw)
    {
        // TODO: draw must be between 0 and 1 

        // Create a discrete_distribution based on the returned weights. This class handles the
        // case where the sum of the weights is < or > 1, by normalizing agains the sum.
        float total = 0.f;
        for (ActionProbability& actionProb : *this)
        { if (actionProb.probability < 0)
            throw std::invalid_argument("Scores must be non-negative.");
          
          total += actionProb.probability;
        }
        if (total == 0)
          throw std::invalid_argument("At least one score must be positive.");

        // TODO: the C# version also checks for sum to 1

        draw *= total;
        if (draw > total) //make very sure that draw can not be greater than total.
          draw = total;

        float action_probability = 0.f;
        size_t action_index = 0;
        float sum = 0.f;
        for (ActionProbability& actionProb : *this)
        {
          sum += actionProb.probability;
          if (sum > draw)
            return action_index;

            action_index++;
        }

        // return the last index
        return size() - 1;
    }

    void ActionProbabilities::swap_first(size_t index)
    {
        iter_swap(begin(), begin() + index);
    }

    void ActionProbabilities::sort_by_probabilities_desc()
    {
        std::sort(begin(), end(), [](const auto& a, const auto& b) {
            return a.probability > b.probability;   
        });
    } 

    // probabilities ordered by action
    vector<float> ActionProbabilities::probabilities()
    {
        vector<float> probs(size());
        for (ActionProbability& actionProb : *this)
            probs[actionProb.action] = actionProb.probability;

        return probs;
    }

    // actions as currently ordered
    vector<int> ActionProbabilities::actions()
    {
        size_t n = size();
        vector<int> actions(n);
        for (size_t i=0;i<n;i++)
            actions[i] = this->operator[](i).action;
            
        return actions;
    }
  }
}