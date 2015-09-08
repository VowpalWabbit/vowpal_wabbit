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
    /// <typeparam name="TSource">The disposable context needed to create objects of <typeparamref name="TObject"/>.</typeparam>
    /// <typeparam name="TObject">The type of the objects to be created.</typeparam>
    public sealed class PooledObject<TSource, TObject> : IDisposable
        where TSource : IDisposable
        where TObject : IDisposable
    {
        private readonly ObjectPool<TSource, TObject> pool;

        internal PooledObject(ObjectPool<TSource, TObject> pool, int version, TObject value)
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
