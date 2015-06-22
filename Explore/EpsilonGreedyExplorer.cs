using System;
using System.Reflection;

namespace MultiWorldTesting
{
    /// <summary>
	/// The epsilon greedy exploration class.
	/// </summary>
	/// <remarks>
	/// This is a good choice if you have no idea which actions should be preferred.
	/// Epsilon greedy is also computationally cheap.
	/// </remarks>
	/// <typeparam name="TContext">The Context type.</typeparam>
	public class EpsilonGreedyExplorer<TContext> : IExplorer<TContext>, IConsumePolicy<TContext>
	{
        private IPolicy<TContext> defaultPolicy;
        private readonly float epsilon;
        private bool explore;
        private readonly uint numActions;

		/// <summary>
		/// The constructor is the only public member, because this should be used with the MwtExplorer.
		/// </summary>
		/// <param name="defaultPolicy">A default function which outputs an action given a context.</param>
		/// <param name="epsilon">The probability of a random exploration.</param>
		/// <param name="numActions">The number of actions to randomize over.</param>
        public EpsilonGreedyExplorer(IPolicy<TContext> defaultPolicy, float epsilon, uint numActions)
		{
            VariableActionHelper.ValidateNumberOfActions(numActions);

            if (epsilon < 0 || epsilon > 1)
            {
                throw new ArgumentException("Epsilon must be between 0 and 1.");
            }
            this.defaultPolicy = defaultPolicy;
            this.epsilon = epsilon;
            this.numActions = numActions;
            this.explore = true;
        }

        /// <summary>
        /// Initializes an epsilon greedy explorer with variable number of actions.
        /// </summary>
        /// <param name="defaultPolicy">A default function which outputs an action given a context.</param>
        /// <param name="epsilon">The probability of a random exploration.</param>
        public EpsilonGreedyExplorer(IPolicy<TContext> defaultPolicy, float epsilon) :
            this(defaultPolicy, epsilon, uint.MaxValue)
        {
            VariableActionHelper.ValidateContextType<TContext>();
        }

        public void UpdatePolicy(IPolicy<TContext> newPolicy)
        {
            this.defaultPolicy = newPolicy;
        }

        public void EnableExplore(bool explore)
        {
            this.explore = explore;
        }

        public DecisionTuple Choose_Action(ulong saltedSeed, TContext context)
        {
            uint numActions = VariableActionHelper.GetNumberOfActions(context, this.numActions);

            var random = new PRG(saltedSeed);

            // Invoke the default policy function to get the action
            uint chosenAction = this.defaultPolicy.ChooseAction(context);

            if (chosenAction == 0 || chosenAction > numActions)
            {
                throw new ArgumentException("Action chosen by default policy is not within valid range.");
            }

            float epsilon = this.explore ? this.epsilon : 0f;

            float actionProbability = 0f;
            float baseProbability = epsilon / numActions; // uniform probability

            if (random.UniformUnitInterval() < 1f - epsilon)
            {
                actionProbability = 1f - epsilon + baseProbability;
            }
            else
            {
                // Get uniform random 1-based action ID
                uint actionId = (uint)random.UniformInt(1, numActions);

                if (actionId == chosenAction)
                {
                    // If it matches the one chosen by the default policy
                    // then increase the probability
                    actionProbability = 1f - epsilon + baseProbability;
                }
                else
                {
                    // Otherwise it's just the uniform probability
                    actionProbability = baseProbability;
                }
                chosenAction = actionId;
            }

            return new DecisionTuple 
            { 
                Action = chosenAction, 
                Probability = actionProbability, 
                ShouldRecord = true 
            };
        }
    };
}
