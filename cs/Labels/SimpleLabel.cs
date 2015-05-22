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
    public class SimpleLabel : ILabel
    {
        public float Label { get; set; }

        public float? Weight { get; set; }

        public float? Initial { get; set; }

        public string ToVowpalWabbitFormat()
        {
            // see simple_label.cc: parse_simple_label

            if (Weight == null)
            {
                return string.Format(CultureInfo.InvariantCulture, "{0}", this.Label);
            }

            if (Initial == null)
            {
                return string.Format(CultureInfo.InvariantCulture, "{0} {1}", this.Label, this.Weight);
            }

            return string.Format("{0} {1} {2}", this.Label, this.Weight, this.Initial);
        }
    }
}
