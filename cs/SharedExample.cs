// --------------------------------------------------------------------------------------------------------------------
// <copyright file="SharedExample.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using Microsoft.Research.MachineLearning.Interfaces;

namespace Microsoft.Research.MachineLearning
{
    /// <summary>
    /// Examples used with <see cref="VowpalWabbit{TExample,TActionDependentFeature}"/> must inherit from this class.
    /// </summary>
    public abstract class SharedExample : IExample
    {
        private static readonly SharedLabel sharedLabel = new SharedLabel();

        public ILabel Label
        {
            get { return sharedLabel; }
        }

        internal class SharedLabel : ILabel
        {
            public string ToVowpalWabbitFormat()
            {
                return "shared";
            }
        }
    }
}
