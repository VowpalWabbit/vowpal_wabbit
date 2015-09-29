// --------------------------------------------------------------------------------------------------------------------
// <copyright file="FeatureExpression.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System;
using System.Linq.Expressions;

namespace VW.Serializer.Intermediate
{
    /// <summary>
    /// Feature data composed during compilation step.
    /// </summary>
    internal sealed class FeatureExpression
    {
        internal Type FeatureType { get; set; }

        internal Type FeatureValueType { get; set; }

        internal string PropertyName { get; set; }

        internal string Name { get; set; }

        internal string Namespace { get; set; }

        internal char? FeatureGroup { get; set; }

        internal bool IsDense { get { return this.DenseFeatureValueElementType != null; } }

        internal bool Enumerize { get; set; }

        internal bool AddAnchor { get; set; }

        internal MemberInitExpression NewFeatureExpression { get; set; }

        internal MemberExpression PropertyExpression { get; set; }

        internal Type DenseFeatureValueElementType { get; set; }

        internal int Order { get; set; }
    }

}
