#include "ds_explore.h"

namespace Microsoft {
  namespace DecisionService {
    IExplorer::~IExplorer()
    { }

    void IExplorer::safety(std::vector<float>& probability_distribution, float min_prob, bool zeros)
    {
        //input: a probability distribution
        //output: a probability distribution with all events having probability > min_prob.  This includes events with probability 0 if zeros = true
        if (min_prob > 0.999) // uniform exploration
        {
            size_t support_size = probability_distribution.size();
            if (!zeros)
            {
                for (float prob : probability_distribution)
                    if (prob == 0)
                        support_size--;
            }
        
            for (float& prob : probability_distribution)
                if (zeros || prob > 0)
                    prob = 1.f / support_size;

            return;
        }

        min_prob /= probability_distribution.size();
        float touched_mass = 0.;
        float untouched_mass = 0.;
        
        for (float& prob : probability_distribution)
        {
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
            // TODO: throw exception
            // if (touched_mass > 0.999)
            //     THROW("Cannot safety this distribution");
            float ratio = (1.f - touched_mass) / untouched_mass;
            for (float& prob : probability_distribution)
                if (prob > min_prob)
                    prob *= ratio;
        }
    }
  }
}