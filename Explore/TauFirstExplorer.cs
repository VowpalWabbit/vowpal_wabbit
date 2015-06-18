
namespace MultiWorldTesting
{
    /// <summary>
	/// The tau-first exploration class.
	/// </summary>
	/// <remarks>
	/// The tau-first explorer collects precisely tau uniform random
	/// exploration events, and then uses the default policy. 
	/// </remarks>
	/// <typeparam name="TContext">The Context type.</typeparam>
    public class TauFirstExplorer<TContext> : IExplorer<TContext>, IConsumePolicy<TContext>
	{
		/// <summary>
		/// The constructor is the only public member, because this should be used with the MwtExplorer.
		/// </summary>
		/// <param name="defaultPolicy">A default policy after randomization finishes.</param>
		/// <param name="tau">The number of events to be uniform over.</param>
		/// <param name="numActions">The number of actions to randomize over.</param>
        public TauFirstExplorer(IPolicy<TContext> defaultPolicy, uint tau, uint numActions)
		{
            // TODO: implement
        }

        /// <summary>
        /// Initializes a tau-first explorer with variable number of actions.
        /// </summary>
        /// <param name="defaultPolicy">A default policy after randomization finishes.</param>
        /// <param name="tau">The number of events to be uniform over.</param>
        public TauFirstExplorer(IPolicy<TContext> defaultPolicy, uint tau)
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
