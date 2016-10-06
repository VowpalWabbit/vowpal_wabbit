// --------------------------------------------------------------------------------------------------------------------
// <copyright file="StringProcessing.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

namespace VW.Serializer
{
    /// <summary>
    /// Options for string pre-processing before feeding to VW native.
    /// </summary>
    public enum StringProcessing
    {
        /// <summary>
        /// Spaces are replaced with underscores.
        /// </summary>
        Escape,

        /// <summary>
        /// Strings are split on space, producing individual features.
        /// </summary>
        Split,

        /// <summary>
        /// Spaces are replaced with underscores and the property name is used as a prefix.
        /// </summary>
        EscapeAndIncludeName
    }
}
