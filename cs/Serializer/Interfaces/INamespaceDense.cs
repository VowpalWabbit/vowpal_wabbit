// --------------------------------------------------------------------------------------------------------------------
// <copyright file="INamespaceDense.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------
using System.Collections.Generic;

namespace VW.Serializer.Interfaces
{
    /// <summary>
    /// A dense namespace.
    /// </summary>
    /// <typeparam name="T">The element type of the dense features.</typeparam>
    public interface INamespaceDense<out T> : INamespace
    {
        /// <summary>
        /// The list of features.
        /// </summary>
        IFeature<IEnumerable<T>> DenseFeature { get; }
    }
}
