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
    public class ContextualBanditLabel : ILabel
    {
        public uint Action { get; set; }

        public float Cost { get; set; }

        public float Probability { get; set; }

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
