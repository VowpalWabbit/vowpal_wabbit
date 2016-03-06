// --------------------------------------------------------------------------------------------------------------------
// <copyright file="VowpalWabbitConstants.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

namespace VW.Serializer
{
    /// <summary>
    /// Constants used throughout C# wrapper.
    /// </summary>
    internal static class VowpalWabbitConstants
    {
        /// <summary>
        /// The VW default namespace is denoted by a blank.
        /// </summary>
        internal const char DefaultNamespace = ' ';

        /// <summary>
        /// JSON properties starting with underscore are ignored.
        /// </summary>
        internal const string FeatureIgnorePrefix = "_";

        /// <summary>
        /// JSON property "_text" is marshalled using <see cref="VW.Serializer.StringProcessing.Split"/>.
        /// </summary>
        internal const string TextProperty = "_text";

        /// <summary>
        /// JSON property "_label" is used as label.
        /// </summary>
        internal const string LabelProperty = "_label";

        /// <summary>
        /// JSON property "_multi" is used to signal multi-line examples.
        /// </summary>
        internal const string MultiProperty = "_multi";

        /// <summary>
        /// True if <paramref name="property"/> is considered a special property and thus should not be skipped.
        /// </summary>
        /// <param name="property">The JSON property name.</param>
        /// <returns>True if <paramref name="property"/> is a special property, false otherwise.</returns>
        internal static bool IsSpecialProperty(string property)
        {
            return property == TextProperty ||
                property == LabelProperty ||
                property == MultiProperty;
        }
    }
}
