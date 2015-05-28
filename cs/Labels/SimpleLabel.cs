// --------------------------------------------------------------------------------------------------------------------
// <copyright file="IVowpalWabbitVisitor.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using Microsoft.Research.MachineLearning.Interfaces;
using System.Globalization;

namespace Microsoft.Research.MachineLearning.Labels
{
    public sealed class SimpleLabel : ILabel
    {
        /// <summary>
        /// Gets or sets the float label.
        /// </summary>
        public float Label { get; set; }

        /// <summary>
        /// Gets or sets the optional weight.
        /// </summary>
        public float? Weight { get; set; }

        /// <summary>
        /// Gets or sets the optional initial value.
        /// </summary>
        public float? Initial { get; set; }

        /// <summary>
        /// Serialize to Vowpal Wabbit string format.
        /// </summary>
        /// <remarks>see simple_label.cc: parse_simple_label</remarks>
        public string ToVowpalWabbitFormat()
        {
            if (Weight == null)
            {
                return string.Format(CultureInfo.InvariantCulture, "{0}", this.Label);
            }

            if (Initial == null)
            {
                return string.Format(CultureInfo.InvariantCulture, "{0} {1}", this.Label, this.Weight);
            }

            return string.Format(CultureInfo.InvariantCulture, "{0} {1} {2}", this.Label, this.Weight, this.Initial);
        }
    }
}
