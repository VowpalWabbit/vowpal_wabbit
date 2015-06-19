// --------------------------------------------------------------------------------------------------------------------
// <copyright file="IVisitableNamespace.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System;

namespace VW.Serializer.Interfaces
{
    /// <summary>
    /// A visitable namespace.
    /// </summary>
    public interface IVisitableNamespace : INamespace
    {
        /// <summary>
        /// Dispatch to the best matching overload of Visit() for this namespace.
        /// </summary>
        Action Visit { get; }
    }
}
