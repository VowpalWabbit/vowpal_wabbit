
namespace MultiWorldTesting
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
		/// <summary>
		/// The constructor is the only public member, because this should be used with the MwtExplorer.
		/// </summary>
		/// <param name="defaultScorer">A function which outputs a score for each action.</param>
		/// <param name="lambda">lambda = 0 implies uniform distribution. Large lambda is equivalent to a max.</param>
		/// <param name="numActions">The number of actions to randomize over.</param>
        public SoftmaxExplorer(IScorer<TContext> defaultScorer, float lambda, uint numActions)
		{
            // TODO: implement
        }

        /// <summary>
        /// Initializes a softmax explorer with variable number of actions.
        /// </summary>
        /// <param name="defaultScorer">A function which outputs a score for each action.</param>
        /// <param name="lambda">lambda = 0 implies uniform distribution. Large lambda is equivalent to a max.</param>
        public SoftmaxExplorer(IScorer<TContext> defaultScorer, float lambda)
        {
            // TODO: implement
        }

        public void UpdateScorer(IScorer<TContext> newScorer)
        {
            // TODO: implement
        }

        public void EnableExplore(bool explore)
        {
            // TODO: implement
        }
	};
}
