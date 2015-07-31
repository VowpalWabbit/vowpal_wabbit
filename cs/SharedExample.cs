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
    public sealed class SharedLabel : ILabel
    {
        public static readonly SharedLabel Singleton = new SharedLabel();

        private SharedLabel()
        {
        }

        public string ToVowpalWabbitFormat()
        {
            return "shared";
        }
    }
}
