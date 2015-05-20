// --------------------------------------------------------------------------------------------------------------------
// <copyright file="IVisitableNamespace.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------
using System;

namespace Microsoft.Research.MachineLearning.Serializer.Interfaces
{
    /// <summary>
    /// A visitable namespace.
    /// </summary>
    /// <typeparam name="TResult">The type of the result produced by the visitor.</typeparam>
    public interface IVisitableNamespace<TNamespaceResult> : INamespace
    {
        /// <summary>
        /// Dispatch to the best matching overload of Visit() for this namespace.
        /// </summary>
        Func<TNamespaceResult> Visit { get; }
    }
}
