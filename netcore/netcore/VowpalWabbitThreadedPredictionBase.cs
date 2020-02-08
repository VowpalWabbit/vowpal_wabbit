// --------------------------------------------------------------------------------------------------------------------
// <copyright file="VowpalWabbitThreadedPredictionBase.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System;
using System.Diagnostics;
using System.Diagnostics.Contracts;

namespace VW
{
    /// <summary>
    /// Enables multi-threaded prediction by utilizing a pool of <see cref="VowpalWabbit"/> instances.
    /// </summary>
    /// <typeparam name="TVowpalWabbit">The VowpalWabbit wrapper type used.</typeparam>
    public abstract class VowpalWabbitThreadedPredictionBase<TVowpalWabbit> : IDisposable
        where TVowpalWabbit : class, IDisposable
    {
        /// <summary>
        /// The pool of potentially wrapped VW instances.
        /// </summary>
        private ObjectPool<VowpalWabbitModel, TVowpalWabbit> vwPool;

        private VowpalWabbitSettings settings;

        /// <summary>
        /// Initializes a new instance of the <see cref="VowpalWabbitThreadedPredictionBase{TVowpalWabbit}"/> class.
        /// </summary>
        /// <param name="model">The initial model to use.</param>
        protected VowpalWabbitThreadedPredictionBase(VowpalWabbitModel model = null)
             : this(new VowpalWabbitSettings() { Model = model })
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="VowpalWabbitThreadedPredictionBase{TVowpalWabbit}"/> class.
        /// </summary>
        /// <param name="settings">The initial settings to use.</param>
        protected VowpalWabbitThreadedPredictionBase(VowpalWabbitSettings settings)
        {
            this.settings = settings;

            this.vwPool = new ObjectPool<VowpalWabbitModel, TVowpalWabbit>(
                ObjectFactory.Create(
                    settings.Model,
                    m =>
                    {
                        if (m == null)
                            return default(TVowpalWabbit);

                        return CreateVowpalWabbitChild(m);
                    }));
        }

        private TVowpalWabbit CreateVowpalWabbitChild(VowpalWabbitModel model)
        {
            var newSettings = (VowpalWabbitSettings)this.settings.Clone();
            newSettings.Model = model;
            var vw = new VowpalWabbit(newSettings);
            return this.InternalCreate(vw);
        }

        /// <summary>
        /// Implementors create new VW wrapper instances.
        /// </summary>
        /// <param name="vw">The native VW instance.</param>
        /// <returns>The new VW wrapper instance.</returns>
        protected abstract TVowpalWabbit InternalCreate(VowpalWabbit vw);

        /// <summary>
        /// Updates the model used for prediction in a thread-safe manner.
        /// </summary>
        /// <param name="model">The new model to be used.</param>
        public void UpdateModel(VowpalWabbitModel model)
        {
            this.vwPool.UpdateFactory(ObjectFactory.Create(
                model,
                this.CreateVowpalWabbitChild));
        }

        /// <summary>
        /// Gets or creates a new VW wrapper instance.
        /// </summary>
        /// <returns>A ready to use VW wrapper instance.</returns>
        /// <remarks><see cref="PooledObject{TModel, TVowpalWabbit}.Value"/> can be null if no model was supplied yet.</remarks>
        public PooledObject<VowpalWabbitModel, TVowpalWabbit> GetOrCreate()
        {
            Contract.Ensures(Contract.Result<PooledObject<VowpalWabbitModel, TVowpalWabbit>>() != null);

            return this.vwPool.GetOrCreate();
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
                if (this.vwPool != null)
                {
                    this.vwPool.Dispose();
                    this.vwPool = null;
                }
            }
        }
    }

}
