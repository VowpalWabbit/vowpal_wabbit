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
    public sealed class PooledObject<TContext, TObject> : IDisposable
        where TContext : IDisposable
        where TObject : IDisposable
    {
        private readonly ObjectPool<TContext, TObject> pool;

        internal PooledObject(ObjectPool<TContext, TObject> pool, int version, TObject value)
        {
            this.pool = pool;
            this.Value = value;
            this.Version = version;
        }

        /// <summary>
        /// The actual value.
        /// </summary>
        public TObject Value { get; private set; }

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
