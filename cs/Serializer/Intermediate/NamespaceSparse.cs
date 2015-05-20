// --------------------------------------------------------------------------------------------------------------------
// <copyright file="NamespaceSparse.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System;
using Microsoft.Research.MachineLearning.Serializer.Interfaces;

namespace Microsoft.Research.MachineLearning.Serializer.Intermediate
{
    public class NamespaceSparse<TNamespaceResult, TFeatureResult> : Namespace, INamespaceSparse<TFeatureResult>, IVisitableNamespace<TNamespaceResult>
    {
        public Func<TNamespaceResult> Visit { get; set; }

        public IVisitableFeature<TFeatureResult>[] Features { get; set; }
    }
}
