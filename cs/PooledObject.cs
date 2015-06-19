// --------------------------------------------------------------------------------------------------------------------
// <copyright file="PooledObject.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System;

namespace VW
{
    /// <summary>
    /// A strongly-typed pooled object.
    /// </summary>
    /// <typeparam name="T">Type of object to pool.</typeparam>
    public sealed class PooledObject<T> : IDisposable
        where T : IDisposable
    {
        private readonly ObjectPool<T> pool;

        internal PooledObject(ObjectPool<T> pool, int version, T value)
        {
            this.pool = pool;
            this.Value = value;
            this.Version = version;
        }

        /// <summary>
        /// The actual value.
        /// </summary>
        public T Value { get; private set; }

        /// <summary>
        /// Factory version used to create Value.
        /// </summary>
        internal int Version { get; private set; }

        /// <summary>
        /// Return to pool.
        /// </summary>
        public void Dispose()
        {
            this.pool.ReturnObject(this);
        }
    }
}
