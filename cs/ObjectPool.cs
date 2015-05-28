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
using Microsoft.Research.MachineLearning.Interfaces;

namespace Microsoft.Research.MachineLearning
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

        public ObjectPool(IObjectFactory<T> factory = null)
        {
            this.rwLockSlim = new ReaderWriterLockSlim();
            this.pool = new Stack<PooledObject<T>>();
            this.factory = factory;
        }

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
                oldFactory = factory;
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
