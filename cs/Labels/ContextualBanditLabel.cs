// --------------------------------------------------------------------------------------------------------------------
// <copyright file="IVowpalWabbitVisitor.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System.Globalization;
using Microsoft.Research.MachineLearning.Interfaces;

namespace Microsoft.Research.MachineLearning.Labels
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
            return string.Format(
                CultureInfo.InvariantCulture, 
                "{0}:{1}:{2}", 
                this.Action, 
                this.Cost, 
                this.Probability);
        }
    }
}
