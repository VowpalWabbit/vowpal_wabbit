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
    /// <typeparam name="TVowpalWabbit">The vowpal wabbit wrapper type.</typeparam>
    public abstract class VowpalWabbitFactoryBase<TVowpalWabbit> : IObjectFactory<TVowpalWabbit>   
        where TVowpalWabbit : VowpalWabbit
    {
        /// <summary>
        /// The shared model.
        /// </summary>
        protected VowpalWabbitModel model;

        /// <summary>
        /// Initializes a new <see cref="VowpalWabbitFactoryBase{TVowpalWabbit}"/> instance.
        /// </summary>
        /// <param name="model">The shared model.</param>
        public VowpalWabbitFactoryBase(VowpalWabbitModel model)
        {
            this.model = model;
        }

        /// <summary>
        /// Creates a new <see cref="VowpalWabbit"/> instance using the shared model.
        /// </summary>
        /// <returns>A new <see cref="VowpalWabbit"/> instance.</returns>
        public abstract TVowpalWabbit Create();

        /// <summary>
        /// Cleanup.
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
                if (model != null)
                {
                    this.model.Dispose();
                    this.model = null;
                }
            }
        }
    }
}
