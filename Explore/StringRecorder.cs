
namespace MultiWorldTesting
{
    /// <summary>
	/// A sample recorder class that converts the exploration tuple into string format.
	/// </summary>
	/// <typeparam name="TContext">The Context type.</typeparam>
	public class StringRecorder<TContext> : IRecorder<TContext>
        where TContext : IStringContext
	{
        public StringRecorder()
		{
            // TODO: implement
		}

        /// <summary>
        /// Records the exploration data associated with a given decision.
        /// This implementation should be thread-safe if multithreading is needed.
        /// </summary>
        /// <param name="context">A user-defined context for the decision.</param>
        /// <param name="action">Chosen by an exploration algorithm given context.</param>
        /// <param name="probability">The probability of the chosen action given context.</param>
        /// <param name="uniqueKey">A user-defined identifer for the decision.</param>
        public void Record(TContext context, uint action, float probability, string uniqueKey)
		{
            // TODO: implement
		}

		/// <summary>
		/// Gets the content of the recording so far as a string and optionally clears internal content.
		/// </summary>
		/// <param name="flush">A boolean value indicating whether to clear the internal content.</param>
		/// <returns>
		/// A string with recording content.
		/// </returns>
        public string GetRecording(bool flush = false)
		{
            // TODO: implement
            return null;
		}
	};
}
