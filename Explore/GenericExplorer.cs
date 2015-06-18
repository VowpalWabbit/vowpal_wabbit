
namespace MultiWorldTesting
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
		/// <summary>
		/// The constructor is the only public member, because this should be used with the MwtExplorer.
		/// </summary>
		/// <param name="defaultScorer">A function which outputs the probability of each action.</param>
		/// <param name="numActions">The number of actions to randomize over.</param>
        public GenericExplorer(IScorer<TContext> defaultScorer, uint numActions)
		{
            // TODO: implement
        }

        /// <summary>
        /// Initializes a generic explorer with variable number of actions.
        /// </summary>
        /// <param name="defaultScorer">A function which outputs the probability of each action.</param>
        public GenericExplorer(IScorer<TContext> defaultScorer)
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
