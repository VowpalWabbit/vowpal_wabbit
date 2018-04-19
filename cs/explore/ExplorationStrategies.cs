using System;
using System.Linq;

namespace Microsoft.DecisionService.Exploration
{
    /// <summary>
    /// Exploration strategies
    /// </summary>
    public static class ExplorationStrategies
    {
        /// <summary>
        /// Generates epsilon-greedy style exploration distribution.
        /// </summary>
        /// <param name="epsilon">probability of exploration each action.</param>
        /// <param name="topAction">Index of the top action receiving 1-epsilon of the probability.</param>
        /// <param name="numActions">Total number of actions.</param>
        /// <returns>The generated exploration distribution.</returns>
        public static float[] GenerateEpsilonGreedy(float epsilon, int topAction, int numActions)
        {
            if (numActions <= 0)
                return new float[0];

            if (topAction >= numActions)
                throw new ArgumentOutOfRangeException("topAction", "topAction must be smaller than numActions");

            float prob = epsilon / numActions;

            var probabilityDistribution = new float[numActions];
            for (int i = 0; i < probabilityDistribution.Length; i++)
                probabilityDistribution[i] = prob;

            probabilityDistribution[topAction] += 1f - epsilon;

            return probabilityDistribution;
        }

        /// <summary>
        /// Generates softmax style exploration distribution.
        /// </summary>
        /// <param name="lambda">Lambda parameter of softmax.</param>
        /// <param name="scores">The scores to use.</param>
        /// <returns>The generated exploration distribution.</returns>
        public static float[] GenerateSoftmax(float lambda, float[] scores)
        {
            float norm = 0;
            float maxScore = scores.Max();

            var probabilityDistribution = new float[scores.Length];
            for (int i = 0; i < probabilityDistribution.Length; i++)
            {
                float prob = (float)Math.Exp(lambda * (scores[i] - maxScore));
                norm += prob;

                probabilityDistribution[i] = prob;
            }

            // normalize
            if (norm > 0)
                for (int i = 0; i < probabilityDistribution.Length; i++)
                    probabilityDistribution[i] /= norm;

            return probabilityDistribution;
        }

        /// <summary>
        /// Generates an exploration distribution according to votes on actions.
        /// </summary>
        /// <param name="topActions">Vote of each model for a given action.</param>
        /// <returns>The generated exploration distribution.</returns>
        public static float[] GenerateBag(int[] topActions)
        {
            if (topActions.Length == 0)
                throw new ArgumentOutOfRangeException("topActions", "must supply at least one topActions from a model");

            // determine probability per model
            float prob = 1f / (float)topActions.Length;

            var probabilityDistribution = new float[topActions.Length];

            for (int i = 0; i < topActions.Length; i++)
                probabilityDistribution[topActions[i]] += prob;

            return probabilityDistribution;
        }

        /// <summary>
        ///  Updates the pdf to ensure each action is explored with at least minimum_uniform/num_actions.
        /// </summary>
        /// <param name="minimum_uniform">The minimum amount of uniform distribution to impose on the pdf.</param>
        /// <param name="update_zero_elements">If true elements with zero probability are updated, otherwise those actions will be unchanged.</param>
        /// <param name="probabilityDistribution">The probability distribution to be modified.</param>
        public static void EnforceMinimumProbability(float minimum_uniform, bool update_zero_elements, float[] probabilityDistribution)
        {
            if (probabilityDistribution.Length == 0)
                throw new ArgumentOutOfRangeException("probabilityDistribution", "probabilityDistribution must have at least 1 element");

            int num_actions = probabilityDistribution.Length;

            if (minimum_uniform > 0.999) // uniform exploration
            {
                int support_size = num_actions;
                if (!update_zero_elements)
                {
                    for (int i = 0; i < probabilityDistribution.Length; i++)
                        if (probabilityDistribution[i] == 0)
                            support_size--;
                }

                for (int i = 0; i < probabilityDistribution.Length; i++)
                    if (update_zero_elements || probabilityDistribution[i] > 0)
                        probabilityDistribution[i] = 1f / support_size;

                return;
            }

            minimum_uniform /= num_actions;
            float touched_mass = 0;
            float untouched_mass = 0;
            int num_actions_touched = 0;

            for (int i = 0; i < probabilityDistribution.Length; i++)
            {
                if ((probabilityDistribution[i] > 0 || (probabilityDistribution[i] == 0 && update_zero_elements)) && probabilityDistribution[i] <= minimum_uniform)
                {
                    touched_mass += minimum_uniform;
                    probabilityDistribution[i] = minimum_uniform;
                    ++num_actions_touched;
                }
                else
                    untouched_mass += probabilityDistribution[i];
            }

            if (touched_mass > 0)
            {
                if (touched_mass > 0.999)
                {
                    minimum_uniform = (1 - untouched_mass) / (float)num_actions_touched;
                    for (int i = 0; i < probabilityDistribution.Length; i++)
                        if ((probabilityDistribution[i] > 0 || (probabilityDistribution[i] == 0 && update_zero_elements)) && probabilityDistribution[i] <= minimum_uniform)
                            probabilityDistribution[i] = minimum_uniform;
                }
                else
                {
                    float ratio = (1 - touched_mass) / untouched_mass;
                    for (int i = 0; i < probabilityDistribution.Length; i++)
                        if (probabilityDistribution[i] > minimum_uniform)
                            probabilityDistribution[i] *= ratio;
                }
            }
        }
    }
}
