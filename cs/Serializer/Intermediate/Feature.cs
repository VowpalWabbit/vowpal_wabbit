// --------------------------------------------------------------------------------------------------------------------
// <copyright file="Feature.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System;
using VW.Serializer.Interfaces;

namespace VW.Serializer.Intermediate
{
    /// <summary>
    /// The intermediate feature representation.
    /// </summary>
    public class Feature : IFeature
    {
        /// <summary>
        /// The targeted namespace.
        /// </summary>
        public string Namespace { get; set; }

        /// <summary>
        /// The targeted feature group.
        /// </summary>
        public char? FeatureGroup { get; set; }

        /// <summary>
        /// The origin property name is used as the feature name.
        /// </summary>
        public string Name { get; set; }

        /// <summary>
        /// If true, features will be converted to string and then hashed.
        /// In VW line format: Age:15 (Enumerize=false), Age_15 (Enumerize=true)
        /// Defaults to false.
        /// </summary>
        public bool Enumerize { get; set; }

        /// <summary>
        /// If true, an anchoring feature (0:1) will be inserted at front.
        /// This is required if --interact is used to mark the beginning of the feature namespace,
        /// as 0-valued features are removed.
        /// </summary>
        /// <remarks>Defaults to false.</remarks>
        public bool AddAnchor { get; set; }
    }

    /// <summary>
    /// The typed representation of the feature.
    /// </summary>
    /// <typeparam name="T">Type of feature value.</typeparam>
    public sealed class Feature<T> : Feature, IFeature<T>, IVisitableFeature
    {
        /// <summary>
        /// The actual value
        /// </summary>
        public T Value { get; set; }

        /// <summary>
        /// Compiled func to enable automatic double dispatch.
        /// </summary>
        public Action Visit { get; set; }
    }
}
