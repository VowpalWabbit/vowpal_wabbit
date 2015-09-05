// --------------------------------------------------------------------------------------------------------------------
// <copyright file="INamespaceDense.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------
using System.Collections.Generic;
using System.Diagnostics.Contracts;
using VW.Serializer.Contracts;

namespace VW.Serializer.Interfaces
{
    /// <summary>
    /// A dense namespace.
    /// </summary>
    /// <typeparam name="T">The element type of the dense features.</typeparam>
    [ContractClass(typeof(INamespaceDenseContract<>))]
    public interface INamespaceDense<out T> // : IMetaNamespace
    {
        /// <summary>
        /// The list of features.
        /// </summary>
        IFeature<IReadOnlyCollection<T>> DenseFeature { get; }
    }
}
