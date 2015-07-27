using System;
using System.Reflection;

namespace MultiWorldTesting.Core
{
    public static class EpsilonGreedy
    {
        public static void Explore(ref uint outAction, ref float outProbability, ref bool outShouldLog,
            uint numActions, bool explore, float defaultEpsilon, ulong saltedSeed)
        {
            var random = new PRG(saltedSeed);
            float epsilon = explore ? defaultEpsilon : 0f;

            float baseProbability = epsilon / numActions; // uniform probability

            if (random.UniformUnitInterval() < 1f - epsilon)
            {
                outProbability = 1f - epsilon + baseProbability;
            }
            else
            {
                // Get uniform random 1-based action ID
                uint actionId = (uint)random.UniformInt(1, numActions);

                if (actionId == outAction)
                {
                    // If it matches the one chosen by the default policy
                    // then increase the probability
                    outProbability = 1f - epsilon + baseProbability;
                }
                else
                {
                    // Otherwise it's just the uniform probability
                    outProbability = baseProbability;
                }
                outAction = actionId;
            }

            outShouldLog = true;
        }
    }
}

namespace MultiWorldTesting.SingleAction
{
    using MultiWorldTesting.Core;

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

            // Invoke the default policy function to get the action
            uint chosenAction = this.defaultPolicy.ChooseAction(context);

            if (chosenAction == 0 || chosenAction > numActions)
            {
                throw new ArgumentException("Action chosen by default policy is not within valid range.");
            }

            float actionProbability = 0f;
            bool shouldRecord = false;

            EpsilonGreedy.Explore(ref chosenAction, ref actionProbability, ref shouldRecord,
                numActions, this.explore, this.epsilon, saltedSeed);

            return new DecisionTuple 
            { 
                Action = chosenAction, 
                Probability = actionProbability,
                ShouldRecord = shouldRecord 
            };
        }
    };
}

namespace MultiWorldTesting.MultiAction
{
    using MultiWorldTesting.Core;

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

        public DecisionTuple ChooseAction(ulong saltedSeed, TContext context)
        {
            uint numActions = VariableActionHelper.GetNumberOfActions(context, this.numActions);

            // Invoke the default policy function to get the action
            uint[] chosenActions = this.defaultPolicy.ChooseAction(context);
            MultiActionHelper.ValidateActionList(chosenActions);

            uint topAction = chosenActions[0];
            float actionProbability = 0f;
            bool shouldRecord = false;

            EpsilonGreedy.Explore(ref topAction, ref actionProbability, ref shouldRecord,
                numActions, this.explore, this.epsilon, saltedSeed);

            // Put chosen action at the top of the list, swapping out the current top.
            MultiActionHelper.PutActionToList(topAction, chosenActions);

            return new DecisionTuple
            {
                Actions = chosenActions,
                Probability = actionProbability,
                ShouldRecord = shouldRecord
            };
        }
    };
}