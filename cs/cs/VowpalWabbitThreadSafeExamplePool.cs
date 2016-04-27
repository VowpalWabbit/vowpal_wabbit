// --------------------------------------------------------------------------------------------------------------------
// <copyright file="VowpalWabbitThreadSafeExamplePool.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System;

namespace VW
{
    public class VowpalWabbitThreadSafeExamplePool : IVowpalWabbitExamplePool
    {
        private IVowpalWabbitExamplePool pool;
        private readonly object poolLock;

        public VowpalWabbitThreadSafeExamplePool(IVowpalWabbitExamplePool pool)
        {
            this.pool = pool;
            this.poolLock = new object();
        }

        public VowpalWabbit Native
        {
            get
            {
                return this.pool.Native;
            }
        }

        public VowpalWabbitExample GetOrCreateNativeExample()
        {
            lock (this.poolLock)
            {
                return this.pool.GetOrCreateNativeExample();
            }
        }

        public void ReturnExampleToPool(VowpalWabbitExample example)
        {
            lock (this.poolLock)
            {
                this.pool.ReturnExampleToPool(example);
            }
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
                if (this.pool != null)
                {
                    this.pool.Dispose();
                    this.pool = null;
                }
            }
        }
    }
}
