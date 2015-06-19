// --------------------------------------------------------------------------------------------------------------------
// <copyright file="IFeature.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

namespace VW.Serializer.Interfaces
{
    /// <summary>
    /// Intermediate representation of a property annotated as <see cref="Microsoft.Research.MachineLearning.Serializer.Attributes.FeatureAttribute"/>.
    /// </summary>
    public interface IFeature
    {
        /// <summary>
        /// The targeted namespace.
        /// </summary>
        string Namespace { get; }

        /// <summary>
        /// The targeted feature group.
        /// </summary>
        char? FeatureGroup { get; }

        /// <summary>
        /// The origin property name is used as the feature name.
        /// </summary>
        string Name { get; }

        /// <summary>
        /// If true, features will be converted to string and then hashed.
        /// In VW line format: Age:15 (Enumerize=false), Age_15 (Enumerize=true)
        /// Defaults to false.
        /// </summary>
        bool Enumerize { get; }
    }

    /// <summary>
    /// The typed representation of the feature.
    /// </summary>
    /// <typeparam name="T">Type of feature value.</typeparam>
    public interface IFeature<out T> : IFeature
    {
        /// <summary>
        /// The actual value of the feature.
        /// </summary>
        T Value { get; }
    }
}
