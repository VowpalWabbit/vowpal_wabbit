// --------------------------------------------------------------------------------------------------------------------
// <copyright file="IVisitableFeature.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System;
using System.Diagnostics.Contracts;
using VW.Serializer.Contracts;

namespace VW.Serializer.Interfaces
{
    /// <summary>
    /// A visitable feature.
    /// </summary>
    [ContractClass(typeof(IVisitableFeatureContract))]
    public interface IVisitableFeature : IFeature
    {
        /// <summary>
        /// Dispatch to the best matching overload of Visit() for this feature.
        /// </summary>
        Action Visit { get; }
    }
}
