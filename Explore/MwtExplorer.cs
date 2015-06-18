using System;

namespace MultiWorldTesting
{
    /// <summary>
	/// The top level MwtExplorer class.  Using this makes sure that the
	/// right bits are recorded and good random actions are chosen.
	/// </summary>
	/// <typeparam name="TContext">The Context type.</typeparam>
	public class MwtExplorer<TContext>
	{
        private int appId;
	    private IRecorder<TContext> recorder;

		/// <summary>
		/// Constructor.
		/// </summary>
		/// <param name="appId">This should be unique to each experiment to avoid correlation bugs.</param>
		/// <param name="recorder">A user-specified class for recording the appropriate bits for use in evaluation and learning.</param>
        public MwtExplorer(string appId, IRecorder<TContext> recorder)
		{
            this.appId = appId.GetHashCode(); // TODO: Int64 hash needed?
            this.recorder = recorder;
        }

		/// <summary>
		/// Choose_Action should be drop-in replacement for any existing policy function.
		/// </summary>
		/// <param name="explorer">An existing exploration algorithm (one of the above) which uses the default policy as a callback.</param>
		/// <param name="uniqueKey">A unique identifier for the experimental unit. This could be a user id, a session id, etc...</param>
		/// <param name="context">The context upon which a decision is made. See SimpleContext above for an example.</param>
		/// <returns>An unsigned 32-bit integer representing the 1-based chosen action.</returns>
        public uint ChooseAction(IExplorer<TContext> explorer, string uniqueKey, TContext context)
        {
            int seed = uniqueKey.GetHashCode();

            ExploreDecision decisionTuple = explorer.Choose_Action(seed + this.appId, context);

            if (decisionTuple.ShouldRecord)
            {
                this.recorder.Record(context, decisionTuple.Action, decisionTuple.Probability, uniqueKey);
            }

            return decisionTuple.Action;
        }
	};
}
