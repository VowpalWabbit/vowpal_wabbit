// --------------------------------------------------------------------------------------------------------------------
// <copyright file="ContextualBanditLabel.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System.Globalization;
using VW.Interfaces;
using System.Text;

namespace VW.Labels
{
    /// <summary>
    /// A contextual bandit label.
    /// </summary>
    public sealed class ContextualBanditLabel : ILabel
    {
        /// <summary>
        /// Gets or sets the action.
        /// </summary>
        public uint Action { get; set; }

        /// <summary>
        /// Gets or sets the cost.
        /// </summary>
        public float Cost { get; set; }

        /// <summary>
        /// Gets or sets the probability with which the action was chosen.
        /// </summary>
        public float Probability { get; set; }

        /// <summary>
        /// Serialize to Vowpal Wabbit string format.
        /// </summary>
        public string ToVowpalWabbitFormat()
        {
            var sb = new StringBuilder();
            
            sb.Append(this.Action.ToString(CultureInfo.InvariantCulture));
            sb.Append(':');
            sb.Append(this.Cost.ToString(CultureInfo.InvariantCulture));
            sb.Append(':');
            sb.Append(this.Probability.ToString(CultureInfo.InvariantCulture));
            
            return sb.ToString();
        }
    }
}
