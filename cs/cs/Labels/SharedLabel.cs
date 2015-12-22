// --------------------------------------------------------------------------------------------------------------------
// <copyright file="SharedLabel.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using VW.Interfaces;

namespace VW.Labels
{
    /// <summary>
    /// In multi-line scenarios the first example can contain a set of shared features. This first example must be 
    /// marked using a 'shared' label.
    /// </summary>
    public sealed class SharedLabel : ILabel
    {
        /// <summary>
        /// The singleton instance .
        /// </summary>
        public static readonly SharedLabel Instance = new SharedLabel();

        private SharedLabel()
        {
        }

        /// <summary>
        /// Label implementation.
        /// </summary>
        public string ToVowpalWabbitFormat()
        {
            return "shared";
        }
    }
}
