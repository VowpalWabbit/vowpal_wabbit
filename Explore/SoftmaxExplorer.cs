using System;
using System.Collections.Generic;
using System.Linq;

namespace MultiWorldTesting.SingleAction
{
    /// <summary>
	/// The softmax exploration class.
	/// </summary>
	/// <remarks>
	/// In some cases, different actions have a different scores, and you
	/// would prefer to choose actions with large scores. Softmax allows 
	/// you to do that.
	/// </remarks>
	/// <typeparam name="TContext">The Context type.</typeparam>
	public class SoftmaxExplorer<TContext> : IExplorer<TContext>, IConsumeScorer<TContext>
	{
        private IScorer<TContext> defaultScorer;
        private bool explore;
	    private readonly float lambda;
        private readonly uint numActions;

		/// <summary>
		/// The constructor is the only public member, because this should be used with the MwtExplorer.
		/// </summary>
		/// <param name="defaultScorer">A function which outputs a score for each action.</param>
		/// <param name="lambda">lambda = 0 implies uniform distribution. Large lambda is equivalent to a max.</param>
		/// <param name="numActions">The number of actions to randomize over.</param>
        public SoftmaxExplorer(IScorer<TContext> defaultScorer, float lambda, uint numActions)
        {
            VariableActionHelper.ValidateNumberOfActions(numActions);

            this.defaultScorer = defaultScorer;
            this.lambda = lambda;
            this.numActions = numActions;
            this.explore = true;
        }

        /// <summary>
        /// Initializes a softmax explorer with variable number of actions.
        /// </summary>
        /// <param name="defaultScorer">A function which outputs a score for each action.</param>
        /// <param name="lambda">lambda = 0 implies uniform distribution. Large lambda is equivalent to a max.</param>
        public SoftmaxExplorer(IScorer<TContext> defaultScorer, float lambda) :
            this(defaultScorer, lambda, uint.MaxValue)
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
            List<float> scores = this.defaultScorer.ScoreActions(context);
            uint numScores = (uint)scores.Count;
            if (scores.Count != numActions)
            {
                throw new ArgumentException("The number of scores returned by the scorer must equal number of actions");
            }

            int i = 0;

            float maxScore = scores.Max();

            float actionProbability = 0f;
            uint actionIndex = 0;
            if (this.explore)
            {
                // Create a normalized exponential distribution based on the returned scores
                for (i = 0; i < numScores; i++)
                {
                    scores[i] = (float)Math.Exp(this.lambda * (scores[i] - maxScore));
                }

                // Create a discrete_distribution based on the returned weights. This class handles the
                // case where the sum of the weights is < or > 1, by normalizing agains the sum.
                float total = scores.Sum();

                float draw = random.UniformUnitInterval();

                float sum = 0f;
                actionProbability = 0f;
                actionIndex = numScores - 1;
                for (i = 0; i < numScores; i++)
                {
                    scores[i] = scores[i] / total;
                    sum += scores[i];
                    if (sum > draw)
                    {
                        actionIndex = (uint)i;
                        actionProbability = scores[i];
                        break;
                    }
                }
            }
            else
            {
                maxScore = 0f;
                for (i = 0; i < numScores; i++)
                {
                    if (maxScore < scores[i])
                    {
                        maxScore = scores[i];
                        actionIndex = (uint)i;
                    }
                }
                actionProbability = 1f; // Set to 1 since we always pick the highest one.
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
    /// The softmax exploration class.
    /// </summary>
    /// <remarks>
    /// In some cases, different actions have a different scores, and you
    /// would prefer to choose actions with large scores. Softmax allows 
    /// you to do that.
    /// </remarks>
    /// <typeparam name="TContext">The Context type.</typeparam>
    public class SoftmaxExplorer<TContext> : IExplorer<TContext>, IConsumeScorer<TContext>
    {
        private IScorer<TContext> defaultScorer;
        private bool explore;
        private readonly float lambda;
        private readonly uint numActions;

        /// <summary>
        /// The constructor is the only public member, because this should be used with the MwtExplorer.
        /// </summary>
        /// <param name="defaultScorer">A function which outputs a score for each action.</param>
        /// <param name="lambda">lambda = 0 implies uniform distribution. Large lambda is equivalent to a max.</param>
        /// <param name="numActions">The number of actions to randomize over.</param>
        public SoftmaxExplorer(IScorer<TContext> defaultScorer, float lambda, uint numActions)
        {
            VariableActionHelper.ValidateNumberOfActions(numActions);

            this.defaultScorer = defaultScorer;
            this.lambda = lambda;
            this.numActions = numActions;
            this.explore = true;
        }

        /// <summary>
        /// Initializes a softmax explorer with variable number of actions.
        /// </summary>
        /// <param name="defaultScorer">A function which outputs a score for each action.</param>
        /// <param name="lambda">lambda = 0 implies uniform distribution. Large lambda is equivalent to a max.</param>
        public SoftmaxExplorer(IScorer<TContext> defaultScorer, float lambda) :
            this(defaultScorer, lambda, uint.MaxValue)
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
            List<float> scores = this.defaultScorer.ScoreActions(context);
            uint numScores = (uint)scores.Count;
            if (scores.Count != numActions)
            {
                throw new ArgumentException("The number of scores returned by the scorer must equal number of actions");
            }

            int i = 0;

            float maxScore = scores.Max();

            uint[] chosenActions = null;
            float actionProbability = 0f;
            if (this.explore)
            {
                // Create a normalized exponential distribution based on the returned scores
                for (i = 0; i < numScores; i++)
                {
                    scores[i] = (float)Math.Exp(this.lambda * (scores[i] - maxScore));
                }

                // Create a discrete_distribution based on the returned weights. This class handles the
                // case where the sum of the weights is < or > 1, by normalizing agains the sum.
                float total = scores.Sum();

                // normalize scores & reset actions
                for (i = 0; i < numScores; i++)
                {
                    scores[i] = scores[i] / total;
                }

                chosenActions = MultiActionHelper.SampleWithoutReplacement(scores, numActions, random, ref actionProbability);
            }
            else
            {
                // avoid linq to optimize perf
                chosenActions = new uint[numActions];
                for (i = 0; i < numActions; i++)
                {
                    chosenActions[i] = (uint)(i + 1);
                }

                uint actionIndex = 0;
                maxScore = 0f;
                for (i = 0; i < numScores; i++)
                {
                    if (maxScore < scores[i])
                    {
                        maxScore = scores[i];
                        actionIndex = (uint)i;
                    }
                }
                // swap max-score action with the first one
                uint firstAction = chosenActions[0];
                chosenActions[0] = chosenActions[actionIndex];
                chosenActions[actionIndex] = firstAction;

                actionProbability = 1f; // Set to 1 since we always pick the highest one.
            }

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
