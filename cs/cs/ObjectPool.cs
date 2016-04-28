// --------------------------------------------------------------------------------------------------------------------
// <copyright file="ObjectPool.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Diagnostics.CodeAnalysis;
using System.Diagnostics.Contracts;
using System.Threading;

namespace VW
{
    /// <summary>
    /// Thread-safe object pool supporting versioned updates.
    /// </summary>
    /// <typeparam name="TSource">The disposable context needed to create objects of <typeparamref name="TObject"/>.</typeparam>
    /// <typeparam name="TObject">The type of the objects to be created.</typeparam>
    public class ObjectPool<TSource, TObject> : IDisposable
        where TSource : class, IDisposable
        where TObject : class, IDisposable
    {
        /// <summary>
        /// Lock resources
        /// </summary>
        private readonly ReaderWriterLockSlim rwLockSlim;

        /// <summary>
        /// Version of the factory function.
        /// </summary>
        private int version;

        /// <summary>
        /// Used to create new pooled objects.
        /// </summary>
        private ObjectFactory<TSource, TObject> factory;

        /// <summary>
        /// The actual pool.
        /// </summary>
        /// <remarks>
        /// To maximize reuse of previously cached items within the pooled objects.
        /// (e.g. cached action dependent features)
        /// </remarks>
        private Stack<PooledObject<TSource, TObject>> pool;

        /// <summary>
        /// Initializes a new ObjectPool.
        /// </summary>
        /// <param name="factory">
        /// An optional factory to create pooled objects on demand.
        /// <see cref="GetOrCreate()"/> will throw if the factory is still null when called.
        /// </param>
        public ObjectPool(ObjectFactory<TSource, TObject> factory = null)
        {
            this.rwLockSlim = new ReaderWriterLockSlim();
            this.pool = new Stack<PooledObject<TSource, TObject>>();
            this.factory = factory;
        }

        /// <summary>
        /// Updates the object factory in a thread-safe manner.
        /// </summary>
        /// <param name="factory">The new object factory to be used.</param>
        public void UpdateFactory(ObjectFactory<TSource, TObject> factory)
        {
            Stack<PooledObject<TSource, TObject>> oldPool;
            ObjectFactory<TSource, TObject> oldFactory;

            this.rwLockSlim.EnterWriteLock();
            try
            {
                if (this.pool == null)
                {
                    throw new ObjectDisposedException("ObjectPool already disposed");
                }

                this.version++;
                oldFactory = this.factory;
                this.factory = factory;
                oldPool = this.pool;
                this.pool = new Stack<PooledObject<TSource, TObject>>();
            }
            finally
            {
                this.rwLockSlim.ExitWriteLock();
            }

            // dispose outdated items
            foreach (var item in oldPool)
            {
                item.Value.Dispose();
            }

            // dispose factory
            if (oldFactory != null)
            {
                oldFactory.Dispose();
            }
        }

        /// <summary>
        /// Returns an instance of TObject from the pool or creates a new instance using the objectFactory
        /// if the pool is empty.
        /// </summary>
        /// <remarks>This method is thread-safe.</remarks>
        public PooledObject<TSource, TObject> GetOrCreate()
        {
            int localVersion;
            ObjectFactory<TSource, TObject> localFactory;

            this.rwLockSlim.EnterUpgradeableReadLock();
            try
            {
                if (this.pool == null)
                {
                    throw new ObjectDisposedException("ObjectPool already disposed");
                }

                if (this.pool.Count == 0)
                {
                    // create a consistent copy
                    localVersion = this.version;
                    localFactory = this.factory;
                }
                else
                {
                    this.rwLockSlim.EnterWriteLock();
                    try
                    {
                        if (this.pool == null)
                        {
                            throw new ObjectDisposedException("ObjectPool already disposed");
                        }

                        return this.pool.Pop();
                    }
                    finally
                    {
                        this.rwLockSlim.ExitWriteLock();
                    }
                }
            }
            finally
            {
                this.rwLockSlim.ExitUpgradeableReadLock();
            }

            if (localFactory == null)
            {
                throw new InvalidOperationException("Factory must be initialized before calling Get()");
            }

            // invoke the factory outside of the lock
            return new PooledObject<TSource, TObject>(this, localVersion, localFactory.Create());
        }

        /// <summary>
        /// Returns <paramref name="pooledObject"/> to the pool of objects, given the version is still the same.
        /// Otherwise <paramref name="pooledObject"/> is disposed.
        /// </summary>
        /// <param name="pooledObject">The object to be returned.</param>
        internal void ReturnObject(PooledObject<TSource, TObject> pooledObject)
        {
            Contract.Ensures(pooledObject != null);

            this.rwLockSlim.EnterUpgradeableReadLock();
            try
            {
                if (this.version == pooledObject.Version && this.pool != null)
                {
                    this.rwLockSlim.EnterWriteLock();
                    try
                    {
                        // double check
                        if (this.version == pooledObject.Version && this.pool != null)
                        {
                            // it's the same version, return to pool
                            this.pool.Push(pooledObject);

                            return;
                        }
                    }
                    finally
                    {
                        this.rwLockSlim.ExitWriteLock();
                    }
                }
            }
            finally
            {
                this.rwLockSlim.ExitUpgradeableReadLock();
            }

            // outdated
            pooledObject.Value.Dispose();
        }

        /// <summary>
        /// Performs application-defined tasks associated with freeing, releasing, or resetting unmanaged resources.
        /// </summary>
        public void Dispose()
        {
            this.Dispose(true);
            GC.SuppressFinalize(this);
        }

        private void Dispose(bool disposing)
        {
            if (disposing)
            {
                this.rwLockSlim.EnterWriteLock();
                try
                {
                    // Dispose pool items
                    if (this.pool != null)
                    {
                        foreach (var item in this.pool)
                        {
                            item.Value.Dispose();
                        }
                        this.pool = null;
                    }

                    // Dispose factory
                    if (this.factory != null)
                    {
                        this.factory.Dispose();
                        this.factory = null;
                    }
                }
                finally
                {
                    this.rwLockSlim.ExitWriteLock();
                }
            }
        }
    }
}