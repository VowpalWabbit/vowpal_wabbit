// --------------------------------------------------------------------------------------------------------------------
// <copyright file="VowpalWabbitFactoryBase.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System;
using VW.Interfaces;

namespace VW
{
    /// <summary>
    /// Base implementation of <see cref="IObjectFactory{T}"/> for Vowpal Wabbit instances spawn of <see cref="VowpalWabbitModel"/>.
    /// </summary>
    /// <typeparam name="T"></typeparam>
    public abstract class VowpalWabbitFactoryBase<T> : IObjectFactory<T>
    {
        protected VowpalWabbitModel model;

        public VowpalWabbitFactoryBase(VowpalWabbitModel model)
        {
            this.model = model;
        }

        public abstract T Create();

        public void Dispose()
        {
            this.Dispose(true);
            GC.SuppressFinalize(this);
        }

        private void Dispose(bool disposing)
        {
            if (disposing)
            {
                if (model != null)
                {
                    this.model.Dispose();
                    this.model = null;
                }
            }
        }
    }
}
