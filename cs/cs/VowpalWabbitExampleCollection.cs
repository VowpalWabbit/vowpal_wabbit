// --------------------------------------------------------------------------------------------------------------------
// <copyright file="VowpalWabbitExampleCollection.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Diagnostics.Contracts;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VW
{
    /// <summary>
    /// Base class for JSON deserialization result.
    /// </summary>
    public abstract class VowpalWabbitExampleCollection : IDisposable
    {
        /// <summary>
        /// The native VW instance.
        /// </summary>
        protected readonly VowpalWabbit vw;

        /// <summary>
        /// Initializes a new instance of the <see cref="VowpalWabbitExampleCollection"/> class.
        /// </summary>
        /// <param name="vw">The VW native instance.</param>
        protected VowpalWabbitExampleCollection(VowpalWabbit vw)
        {
            Contract.Requires(vw != null);
            this.vw = vw;
        }

        /// <summary>
        /// Learns from this example.
        /// </summary>
        public abstract void Learn();

        /// <summary>
        /// Predicts for this example.
        /// </summary>
        public abstract void Predict();

        /// <summary>
        /// Learn from this example and returns the current prediction for it.
        /// </summary>
        /// <typeparam name="TPrediction">The prediction type.</typeparam>
        /// <param name="predictionFactory">The prediction factory to be used. See <see cref="VowpalWabbitPredictionType"/>.</param>
        /// <returns>The prediction for the this example.</returns>
        public abstract TPrediction Learn<TPrediction>(IVowpalWabbitPredictionFactory<TPrediction> predictionFactory);

        /// <summary>
        /// Predicts for this example and returns the current prediction for it.
        /// </summary>
        /// <typeparam name="TPrediction">The prediction type.</typeparam>
        /// <param name="predictionFactory">The prediction factory to be used. See <see cref="VowpalWabbitPredictionType"/>.</param>
        /// <returns>The prediction for the this example.</returns>
        public abstract TPrediction Predict<TPrediction>(IVowpalWabbitPredictionFactory<TPrediction> predictionFactory);

        /// <summary>
        /// The optional string version of the example.
        /// </summary>
        public abstract string VowpalWabbitString { get; }

        /// <summary>
        /// Performs application-defined tasks associated with freeing, releasing, or resetting unmanaged resources.
        /// </summary>
        public abstract void Dispose();
    }
}
