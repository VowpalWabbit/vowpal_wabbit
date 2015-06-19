// --------------------------------------------------------------------------------------------------------------------
// <copyright file="NamespaceSparse.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System;
using VW.Serializer.Interfaces;

namespace VW.Serializer.Intermediate
{
    /// <summary>
    /// The intermediate representation of a sparse namespace.
    /// </summary>
    public sealed class NamespaceSparse : Namespace, INamespaceSparse, IVisitableNamespace
    {
        public Action Visit { get; set; }

        public IVisitableFeature[] Features { get; set; }
    }
}
