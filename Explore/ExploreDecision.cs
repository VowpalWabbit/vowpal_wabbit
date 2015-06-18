
namespace MultiWorldTesting
{
    /// <summary>
    /// Exploration result 
    /// </summary>
    public class DecisionTuple
    {
        /// <summary>
        /// Action chosen by exploration.
        /// </summary>
        public uint Action { get; set; }

        /// <summary>
        /// Probability of choosing the action.
        /// </summary>
        public float Probability { get; set; }

        /// <summary>
        /// Whether to record/log the exploration result. 
        /// </summary>
        public bool ShouldRecord { get; set; }
    }
}
