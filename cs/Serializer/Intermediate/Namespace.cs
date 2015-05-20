// --------------------------------------------------------------------------------------------------------------------
// <copyright file="Namespace.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------
using Microsoft.Research.MachineLearning.Serializer.Interfaces;

namespace Microsoft.Research.MachineLearning.Serializer.Intermediate
{
    public abstract class Namespace : INamespace
    {
        /// <summary>
        /// The namespace name.
        /// </summary>
        public string Name { get; set; }

        public char? FeatureGroup { get; set; }
    }
}
