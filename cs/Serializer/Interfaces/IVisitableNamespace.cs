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
    public interface IVisitableNamespace<TNamespaceResult> : INamespace
    {
        Func<TNamespaceResult> Visit { get; }
    }
}
