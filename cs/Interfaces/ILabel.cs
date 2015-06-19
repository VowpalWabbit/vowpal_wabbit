// --------------------------------------------------------------------------------------------------------------------
// <copyright file="ILabel.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

namespace VW.Interfaces
{
    /// <summary>
    /// Interface implemented by all valid VowpalWabbit labels.
    /// </summary>
    public interface ILabel
    {
        /// <summary>
        /// Serialize to Vowpal Wabbit string format.
        /// </summary>
        string ToVowpalWabbitFormat();
    }
}
