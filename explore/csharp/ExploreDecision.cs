
namespace MultiWorldTesting.Core
{
    /// <summary>
    /// Exploration result 
    /// </summary>
    public class BaseDecisionTuple
    {
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

namespace MultiWorldTesting.SingleAction
{
    using MultiWorldTesting.Core;

    /// <summary>
    /// Exploration result 
    /// </summary>
    public class DecisionTuple : BaseDecisionTuple
    {
        /// <summary>
        /// Action chosen by exploration.
        /// </summary>
        public uint Action { get; set; }
    }
}

namespace MultiWorldTesting.MultiAction
{
    using MultiWorldTesting.Core;

    /// <summary>
    /// Exploration result 
    /// </summary>
    public class DecisionTuple : BaseDecisionTuple
    {
        /// <summary>
        /// List of actions chosen by exploration.
        /// </summary>
        public uint[] Actions { get; set; }
    }
}
