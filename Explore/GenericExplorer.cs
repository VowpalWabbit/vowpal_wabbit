using System;
using System.Collections.Generic;

namespace MultiWorldTesting.SingleAction
{
    /// <summary>
	/// The generic exploration class.
	/// </summary>
	/// <remarks>
	/// GenericExplorer provides complete flexibility.  You can create any
	/// distribution over actions desired, and it will draw from that.
	/// </remarks>
	/// <typeparam name="TContext">The Context type.</typeparam>
    public class GenericExplorer<TContext> : IExplorer<TContext>, IConsumeScorer<TContext>
	{
        private IScorer<TContext> defaultScorer;
        private bool explore;
        private readonly uint numActions;

		/// <summary>
		/// The constructor is the only public member, because this should be used with the MwtExplorer.
		/// </summary>
		/// <param name="defaultScorer">A function which outputs the probability of each action.</param>
		/// <param name="numActions">The number of actions to randomize over.</param>
        public GenericExplorer(IScorer<TContext> defaultScorer, uint numActions)
		{
            VariableActionHelper.ValidateNumberOfActions(numActions);

            this.defaultScorer = defaultScorer;
            this.numActions = numActions;
            this.explore = true;
        }

        /// <summary>
        /// Initializes a generic explorer with variable number of actions.
        /// </summary>
        /// <param name="defaultScorer">A function which outputs the probability of each action.</param>
        public GenericExplorer(IScorer<TContext> defaultScorer) :
            this(defaultScorer, uint.MaxValue)
        {
            VariableActionHelper.ValidateContextType<TContext>();
        }

        public void UpdateScorer(IScorer<TContext> newScorer)
        {
            this.defaultScorer = newScorer;
        }

        public void EnableExplore(bool explore)
        {
            this.explore = explore;
        }

        public DecisionTuple Choose_Action(ulong saltedSeed, TContext context)
        {
            uint numActions = VariableActionHelper.GetNumberOfActions(context, this.numActions);

            var random = new PRG(saltedSeed);

            // Invoke the default scorer function
            List<float> weights = this.defaultScorer.ScoreActions(context);
            uint numWeights = (uint)weights.Count;
            if (numWeights != numActions)
            {
                throw new ArgumentException("The number of weights returned by the scorer must equal number of actions");
            }

            // Create a discrete_distribution based on the returned weights. This class handles the
            // case where the sum of the weights is < or > 1, by normalizing agains the sum.
            float total = 0f;
            for (int i = 0; i < numWeights; i++)
            {
                if (weights[i] < 0)
                {
                    throw new ArgumentException("Scores must be non-negative.");
                }
                total += weights[i];
            }
            if (total == 0)
            {
                throw new ArgumentException("At least one score must be positive.");
            }

            float draw = random.UniformUnitInterval();

            float sum = 0f;
            float actionProbability = 0f;
            uint actionIndex = numWeights - 1;
            for (int i = 0; i < numWeights; i++)
            {
                weights[i] = weights[i] / total;
                sum += weights[i];
                if (sum > draw)
                {
                    actionIndex = (uint)i;
                    actionProbability = weights[i];
                    break;
                }
            }

            // action id is one-based
            return new DecisionTuple
            {
                Action = actionIndex + 1,
                Probability = actionProbability,
                ShouldRecord = true
            };
        }
    };
}

namespace MultiWorldTesting.MultiAction
{
    /// <summary>
    /// The generic exploration class.
    /// </summary>
    /// <remarks>
    /// GenericExplorer provides complete flexibility.  You can create any
    /// distribution over actions desired, and it will draw from that.
    /// </remarks>
    /// <typeparam name="TContext">The Context type.</typeparam>
    public class GenericExplorer<TContext> : IExplorer<TContext>, IConsumeScorer<TContext>
    {
        private IScorer<TContext> defaultScorer;
        private bool explore;
        private readonly uint numActions;

        /// <summary>
        /// The constructor is the only public member, because this should be used with the MwtExplorer.
        /// </summary>
        /// <param name="defaultScorer">A function which outputs the probability of each action.</param>
        /// <param name="numActions">The number of actions to randomize over.</param>
        public GenericExplorer(IScorer<TContext> defaultScorer, uint numActions)
        {
            VariableActionHelper.ValidateNumberOfActions(numActions);

            this.defaultScorer = defaultScorer;
            this.numActions = numActions;
            this.explore = true;
        }

        /// <summary>
        /// Initializes a generic explorer with variable number of actions.
        /// </summary>
        /// <param name="defaultScorer">A function which outputs the probability of each action.</param>
        public GenericExplorer(IScorer<TContext> defaultScorer) :
            this(defaultScorer, uint.MaxValue)
        {
            VariableActionHelper.ValidateContextType<TContext>();
        }

        public void UpdateScorer(IScorer<TContext> newScorer)
        {
            this.defaultScorer = newScorer;
        }

        public void EnableExplore(bool explore)
        {
            this.explore = explore;
        }

        public DecisionTuple ChooseAction(ulong saltedSeed, TContext context)
        {
            uint numActions = VariableActionHelper.GetNumberOfActions(context, this.numActions);

            var random = new PRG(saltedSeed);

            // Invoke the default scorer function
            List<float> weights = this.defaultScorer.ScoreActions(context);
            uint numWeights = (uint)weights.Count;
            if (numWeights != numActions)
            {
                throw new ArgumentException("The number of weights returned by the scorer must equal number of actions");
            }

            // Create a discrete_distribution based on the returned weights. This class handles the
            // case where the sum of the weights is < or > 1, by normalizing agains the sum.
            float total = 0f;
            for (int i = 0; i < numWeights; i++)
            {
                if (weights[i] < 0)
                {
                    throw new ArgumentException("Scores must be non-negative.");
                }
                total += weights[i];
            }
            if (total == 0)
            {
                throw new ArgumentException("At least one score must be positive.");
            }

            // normalize weights & reset actions
            for (int i = 0; i < numWeights; i++)
            {
                weights[i] = weights[i] / total;
            }

            float actionProbability = 0f;
            uint[] chosenActions = MultiActionHelper.SampleWithoutReplacement(weights, numActions, random, ref actionProbability);

            // action id is one-based
            return new DecisionTuple
            {
                Actions = chosenActions,
                Probability = actionProbability,
                ShouldRecord = true
            };
        }
    };
}
