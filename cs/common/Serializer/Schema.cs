// --------------------------------------------------------------------------------------------------------------------
// <copyright file="FeatureExpression.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System.Collections.Generic;

namespace VW.Serializer
{
    /// <summary>
    /// Describes the serializatoin for a give type.
    /// </summary>
    public sealed class Schema
    {
        /// <summary>
        /// List of features to extract from type.
        /// </summary>
        public List<FeatureExpression> Features { get; set; }

        /// <summary>
        /// Expression to access the label.
        /// </summary>
        public LabelExpression Label { get; set; }
    }
}
