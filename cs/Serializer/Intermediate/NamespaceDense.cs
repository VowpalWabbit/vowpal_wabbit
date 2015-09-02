// --------------------------------------------------------------------------------------------------------------------
// <copyright file="NamespaceDense.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System;
using System.Collections.Generic;
using VW.Serializer.Interfaces;

namespace VW.Serializer.Intermediate
{
    /// <summary>
    /// The intermediate representation of a dense namespace.
    /// </summary>
    /// <typeparam name="TFeature">The element type of the dense features.</typeparam>
    public sealed class NamespaceDense<TFeature> : Namespace, INamespaceDense<TFeature>, IVisitableNamespace
    {
        /// <summary>
        /// Invoke to dispatch to best matched method.
        /// </summary>
        public Action Visit { get; set; }

        /// <summary>
        /// The dense feature data.
        /// </summary>
        public IFeature<IReadOnlyCollection<TFeature>> DenseFeature { get; set; }
    }
}
