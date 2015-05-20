// --------------------------------------------------------------------------------------------------------------------
// <copyright file="IVisitableFeature.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------
using System;

namespace Microsoft.Research.MachineLearning.Serializer.Interfaces
{
    /// <summary>
    /// A visitable feature.
    /// </summary>
    /// <typeparam name="TResult">The type of the result produced by the visitor.</typeparam>
    public interface IVisitableFeature<out TResult> : IFeature
    {
        /// <summary>
        /// Dispatch to the best matching overload of Visit() for this feature.
        /// </summary>
        Func<TResult> Visit { get; }
    }
}
