
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
		/// <summary>
		/// The constructor is the only public member, because this should be used with the MwtExplorer.
		/// </summary>
		/// <param name="defaultPolicy">A default function which outputs an action given a context.</param>
		/// <param name="epsilon">The probability of a random exploration.</param>
		/// <param name="numActions">The number of actions to randomize over.</param>
        public EpsilonGreedyExplorer(IPolicy<TContext> defaultPolicy, float epsilon, uint numActions)
		{
            // TODO: implement
        }

        /// <summary>
        /// Initializes an epsilon greedy explorer with variable number of actions.
        /// </summary>
        /// <param name="defaultPolicy">A default function which outputs an action given a context.</param>
        /// <param name="epsilon">The probability of a random exploration.</param>
        public EpsilonGreedyExplorer(IPolicy<TContext> defaultPolicy, float epsilon)
        {
            // TODO: implement
        }

        public void UpdatePolicy(IPolicy<TContext> newPolicy)
        {
            // TODO: implement
        }

        public void EnableExplore(bool explore)
        {
            // TODO: implement
        }
	};
}
