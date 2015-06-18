
namespace MultiWorldTesting
{
    /// <summary>
	/// The bootstrap exploration class.
	/// </summary>
	/// <remarks>
	/// The Bootstrap explorer randomizes over the actions chosen by a set of
	/// default policies.  This performs well statistically but can be
	/// computationally expensive.
	/// </remarks>
	/// <typeparam name="TContext">The Context type.</typeparam>
	public class BootstrapExplorer<TContext> : IExplorer<TContext>, IConsumePolicies<TContext>
	{
		/// <summary>
		/// The constructor is the only public member, because this should be used with the MwtExplorer.
		/// </summary>
		/// <param name="defaultPolicies">A set of default policies to be uniform random over.</param>
		/// <param name="numActions">The number of actions to randomize over.</param>
        public BootstrapExplorer(IPolicy<TContext>[] defaultPolicies, uint numActions)
		{
            // TODO: implement
        }

        /// <summary>
        /// Initializes a bootstrap explorer with variable number of actions.
        /// </summary>
        /// <param name="defaultPolicies">A set of default policies to be uniform random over.</param>
        public BootstrapExplorer(IPolicy<TContext>[] defaultPolicies)
        {
            // TODO: implement
        }

        public void UpdatePolicy(IPolicy<TContext>[] newPolicies)
        {
            // TODO: implement
        }

        public void EnableExplore(bool explore)
        {
            // TODO: implement
        }

        public ExploreDecision Choose_Action(long saltedSeed, TContext context)
        {
            // TODO: implement
            return null;
        }
    };
}
