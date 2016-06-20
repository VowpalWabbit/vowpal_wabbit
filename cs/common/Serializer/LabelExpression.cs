// --------------------------------------------------------------------------------------------------------------------
// <copyright file="LabelExpression.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Linq.Expressions;

namespace VW.Serializer
{
    /// <summary>
    /// Defines access to the label for an user-specified example type.
    /// </summary>
    public sealed class LabelExpression
    {
        /// <summary>
        /// The name of the label.
        /// </summary>
        public string Name { get; set; }

        /// <summary>
        /// The type of the feature.
        /// </summary>
        public Type LabelType { get; set; }

        /// <summary>
        /// Factory to extract the value for a given feature from the example object (input argument).
        /// </summary>
        public Func<Expression, Expression> ValueExpressionFactory { get; set; }

        /// <summary>
        /// Factories to provide validation before invoking the expression created through <see cref="ValueExpressionFactory"/>.
        /// </summary>
        public List<Func<Expression, Expression>> ValueValidExpressionFactories { get; set; }

    }
}
