// --------------------------------------------------------------------------------------------------------------------
// <copyright file="VowpalWabbitFactory.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.IO;
using System.Threading.Tasks;
using VW.Interfaces;
using VW.Serializer;

namespace VW
{
    /// <summary>
    /// Enables multi-threaded prediction by utilizing a pool of <see cref="VowpalWabbit"/> instances.  
    /// </summary>
    /// <typeparam name="TVowpalWabbit">The VowpalWabbit wrapper type used.</typeparam>
    public abstract class VowpalWabbitThreadedPredictionBase<TVowpalWabbit> : IDisposable
        where TVowpalWabbit : IDisposable
    {
        /// <summary>
        /// The pool of potentially wrapped VW instances.
        /// </summary>
        private ObjectPool<VowpalWabbitModel, TVowpalWabbit> vwPool;

        protected VowpalWabbitThreadedPredictionBase(VowpalWabbitModel model)
        {
            this.vwPool = new ObjectPool<VowpalWabbitModel, TVowpalWabbit>(
                ObjectFactory.Create(
                    model, 
                    m => this.InternalCreate(new VowpalWabbit(m.Settings.ShallowCopy(model: m)))));
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
                m => this.InternalCreate(new VowpalWabbit(m.Settings.ShallowCopy(model: m)))));
        }

        /// <summary>
        /// Gets or creates a new VW wrapper instance.
        /// </summary>
        /// <returns>A ready to use VW wrapper instance</returns>
        public PooledObject<VowpalWabbitModel, TVowpalWabbit> GetOrCreate()
        {
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

    /// <summary>
    /// Enables multi-threaded prediction by utilizing a pool of <see cref="VowpalWabbit"/> instances.  
    /// </summary>
    /// <typeparam name="TExample">The type use for providing data to VW using the serializer infrastructure.</typeparam>
    public class VowpalWabbitThreadedPrediction<TExample> : VowpalWabbitThreadedPredictionBase<VowpalWabbit<TExample>>
    {
        /// <summary>
        /// Initializes a new instance of <see cref="VowpalWabbitThreadedPrediction"/>.
        /// </summary>
        /// <param name="model">The model used by each pool instance.</param>
        public VowpalWabbitThreadedPrediction(VowpalWabbitModel model)
            : base(model)
        {
        }

        /// <summary>
        /// Creates a new instance of <see cref="VowpalWabbit{TExample}"/>.
        /// </summary>
        /// <param name="vw">The wrapped vw instance.</param>
        sealed protected override VowpalWabbit<TExample> InternalCreate(VowpalWabbit vw)
        {
            return new VowpalWabbit<TExample>(vw);
        }
    }

    /// <summary>
    /// Enables multi-threaded prediction by utilizing a pool of <see cref="VowpalWabbit"/> instances.  
    /// </summary>
    /// <typeparam name="TExample">The type use for providing data to VW using the serializer infrastructure.</typeparam>
    /// <typeparam name="TActionDependentFeature">The type use for providing action dependent data to VW using the serializer infrastructure.</typeparam>
    public class VowpalWabbitThreadedPrediction<TExample, TActionDependentFeature> : VowpalWabbitThreadedPredictionBase<VowpalWabbit<TExample, TActionDependentFeature>>
    {
        public VowpalWabbitThreadedPrediction(VowpalWabbitModel model)
            : base(model)
        {
        }

        /// <summary>
        /// Creates a new instance of <see cref="VowpalWabbit{TExample}"/>.
        /// </summary>
        /// <param name="vw">The wrapped vw instance.</param>
        sealed protected override VowpalWabbit<TExample, TActionDependentFeature> InternalCreate(VowpalWabbit vw)
        {
            return new VowpalWabbit<TExample, TActionDependentFeature>(vw);
        }
    }
}
