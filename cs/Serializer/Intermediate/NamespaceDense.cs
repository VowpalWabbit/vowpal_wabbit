// --------------------------------------------------------------------------------------------------------------------
// <copyright file="NamespaceDense.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System;
using System.Collections.Generic;
using Microsoft.Research.MachineLearning.Serializer.Interfaces;

namespace Microsoft.Research.MachineLearning.Serializer.Intermediate
{
    public class NamespaceDense<TFeature, TNamespaceResult> : Namespace, INamespaceDense<TFeature>, IVisitableNamespace<TNamespaceResult>
    {
        public Func<TNamespaceResult> Visit { get; set; }

        public IFeature<IEnumerable<TFeature>> DenseFeature { get; set; }
    }
}
