// --------------------------------------------------------------------------------------------------------------------
// <copyright file="SharedExample.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using VW.Interfaces;

namespace VW
{
    /// <summary>
    /// Examples used with <see cref="VowpalWabbit{TExample,TActionDependentFeature}"/> must inherit from this class.
    /// In multi-line scenarios the first example can contain a set of shared features. This first example must be 
    /// marked using a 'shared' label.
    /// </summary>
    public abstract class SharedExample : IExample
    {
        private static readonly SharedLabel sharedLabel = new SharedLabel();

        /// <summary>
        /// Gets the fixed label required for multi-line examples.
        /// </summary>
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
