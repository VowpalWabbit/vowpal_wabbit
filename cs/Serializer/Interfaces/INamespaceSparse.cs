// --------------------------------------------------------------------------------------------------------------------
// <copyright file="INamespaceSparse.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

namespace VW.Serializer.Interfaces
{
    /// <summary>
    /// A sparse namespace.
    /// </summary>
    public interface INamespaceSparse : INamespace
    {
        /// <summary>
        /// The features of the namespace.
        /// </summary>
        IVisitableFeature[] Features { get; }
    }
}
