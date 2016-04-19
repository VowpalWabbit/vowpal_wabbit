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
        where TSource : class, IDisposable
        where TObject : class, IDisposable
    {
        /// <summary>
        /// The parent pool.
        /// </summary>
        private readonly ObjectPool<TSource, TObject> pool;

        /// <summary>
        /// Initializes a new instance of the <see cref="PooledObject{TSource,TObject}"/> class.
        /// </summary>
        /// <param name="pool">The parent pool.</param>
        /// <param name="version">The version of the pool at time of creation of this instance.</param>
        /// <param name="value">The actual pooled object.</param>
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
            // don't keep empty objects in pool
            if (this.Value != null)
                this.pool.ReturnObject(this);
        }
    }
}
