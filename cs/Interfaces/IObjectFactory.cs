// --------------------------------------------------------------------------------------------------------------------
// <copyright file="IObjectFactory.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System;

namespace VW.Interfaces
{
    /// <summary>
    /// Disposable object factory.
    /// </summary>
    /// <typeparam name="T">The type of the objects to be created.</typeparam>
    public interface IObjectFactory<T> : IDisposable
    {
        /// <summary>
        /// Creates a new object of type T.
        /// </summary>
        T Create();
    }
}
