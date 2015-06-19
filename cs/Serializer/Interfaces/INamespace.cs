// --------------------------------------------------------------------------------------------------------------------
// <copyright file="INamespace.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

namespace VW.Serializer.Interfaces
{
    /// <summary>
    /// Intermediate representation of a namespace.
    /// </summary>
    public interface INamespace
    {
        /// <summary>
        /// The namespace name.
        /// </summary>
        string Name { get; }

        /// <summary>
        /// The regular VW string interface interprets the first character of the namespace as the feature group. 
        /// </summary>
        char? FeatureGroup { get; }
    }
}
