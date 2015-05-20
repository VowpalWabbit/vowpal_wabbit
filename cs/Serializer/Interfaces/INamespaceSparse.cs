// --------------------------------------------------------------------------------------------------------------------
// <copyright file="INamespaceSparse.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

namespace Microsoft.Research.MachineLearning.Serializer.Interfaces
{
    /// <summary>
    /// A sparse namespace.
    /// </summary>
    /// <typeparam name="TResultFeature"></typeparam>
    public interface INamespaceSparse<TResultFeature> : INamespace
    {
        /// <summary>
        /// The features of the namespace.
        /// </summary>
        IVisitableFeature<TResultFeature>[] Features { get; }
    }
}
