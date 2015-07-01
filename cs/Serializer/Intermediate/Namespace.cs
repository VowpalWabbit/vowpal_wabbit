// --------------------------------------------------------------------------------------------------------------------
// <copyright file="Namespace.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------
using VW.Serializer.Interfaces;

namespace VW.Serializer.Intermediate
{
    /// <summary>
    /// The intermediate namespace representation.
    /// </summary>
    public abstract class Namespace : INamespace
    {
        /// <summary>
        /// Gets or sets the namespace name.
        /// </summary>
        public string Name { get; set; }

        /// <summary>
        /// Gets or sets the feature group.
        /// </summary>
        public char? FeatureGroup { get; set; }
    }
}
