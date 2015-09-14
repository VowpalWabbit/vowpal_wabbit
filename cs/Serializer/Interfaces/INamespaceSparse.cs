// --------------------------------------------------------------------------------------------------------------------
// <copyright file="INamespaceSparse.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System.Diagnostics.Contracts;
using VW.Serializer.Contracts;

namespace VW.Serializer.Interfaces
{
    /// <summary>
    /// A sparse namespace.
    /// </summary>
    [ContractClass(typeof(INamespaceSparseContract))]
    public interface INamespaceSparse : INamespace
    {
        /// <summary>
        /// The features of the namespace.
        /// </summary>
        IVisitableFeature[] Features { get; }
    }
}
