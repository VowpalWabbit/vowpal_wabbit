// --------------------------------------------------------------------------------------------------------------------
// <copyright file="ObjectPool.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Threading;
using VW.Interfaces;

namespace VW
{
    /// <summary>
    /// Thread-safe object pool supporting versioned updates.
    /// </summary>
    public class ObjectPool<T> : IDisposable
        where T : IDisposable
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
        private IObjectFactory<T> factory;

        /// <summary>
        /// The actual pool.
        /// </summary>
        /// <remarks>
        /// To maximize reuse of previously cached items within the pooled objects.
        /// (e.g. cached action dependent features)
        /// </remarks>
        private Stack<PooledObject<T>> pool; 

        /// <summary>
        /// Initializes a new ObjectPool.
        /// </summary>
        /// <param name="factory">
        /// An optional factory to create pooled objects on demand. 
        /// <see cref="Get()"/> will throw if the factory is still null when called.
        /// </param>
        public ObjectPool(IObjectFactory<T> factory = null)
        {
            this.rwLockSlim = new ReaderWriterLockSlim();
            this.pool = new Stack<PooledObject<T>>();
            this.factory = factory;
        }

        /// <summary>
        /// Updates the object factory in a thread-safe manner.
        /// </summary>
        /// <param name="factory">The new object factory to be used.</param>
        public void UpdateFactory(IObjectFactory<T> factory)
        {
            Stack<PooledObject<T>> oldPool;
            IObjectFactory<T> oldFactory;

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
                this.pool = new Stack<PooledObject<T>>();
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
        /// Returns an instance of T from the pool or creates a new instance using the objectFactory
        /// if the pool is empty.
        /// </summary>
        /// <remarks>This method is thread-safe.</remarks>
        public PooledObject<T> Get()
        {
            int localVersion;
            IObjectFactory<T> localFactory;

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
            return new PooledObject<T>(this, localVersion, localFactory.Create());
        }

        internal void ReturnObject(PooledObject<T> pooledObject)
        {
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
