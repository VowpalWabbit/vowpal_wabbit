// --------------------------------------------------------------------------------------------------------------------
// <copyright file="IExample.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

namespace VW.Interfaces
{
    /// <summary>
    /// User classes that support labeling need to implement this interface.
    /// </summary>
    public interface IExample
    {
        /// <summary>
        /// The label that this example is tagged with.
        /// </summary>
        ILabel Label { get; }
    }
}
