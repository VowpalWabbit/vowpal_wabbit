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
using VW.Labels;

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
        private readonly VowpalWabbit vw;

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
        /// Learns this example on the VW instance used for marshalling or the optionally passed on <paramref name="vw"/>.
        /// </summary>
        /// <param name="vw">The optional VW instance used for learning. Defaults to the one used for marshalling.</param>
        public void Learn(VowpalWabbit vw = null)
        {
            this.LearnInternal(vw ?? this.vw);
        }

        /// <summary>
        /// Predicts for this example.
        /// </summary>
        /// <param name="vw">Use this VW instance for prediction instead of the one the example was created from.</param>
        public void Predict(VowpalWabbit vw = null)
        {
            this.PredictInternal(vw ?? this.vw);
        }

        /// <summary>
        /// Learn from this example and returns the current prediction for it.
        /// </summary>
        /// <typeparam name="TPrediction">The prediction type.</typeparam>
        /// <param name="predictionFactory">The prediction factory to be used. See <see cref="VowpalWabbitPredictionType"/>.</param>
        /// <returns>The prediction for the this example.</returns>
        /// <param name="vw">Use this VW instance for learning instead of the one the example was created from.</param>
        public TPrediction Learn<TPrediction>(IVowpalWabbitPredictionFactory<TPrediction> predictionFactory, VowpalWabbit vw = null)
        {
            return this.LearnInternal(predictionFactory, vw ?? this.vw);
        }

        /// <summary>
        /// Predicts for this example and returns the current prediction for it.
        /// </summary>
        /// <typeparam name="TPrediction">The prediction type.</typeparam>
        /// <param name="predictionFactory">The prediction factory to be used. See <see cref="VowpalWabbitPredictionType"/>.</param>
        /// <returns>The prediction for the this example.</returns>
        /// <param name="vw">Use this VW instance for prediction instead of the one the example was created from.</param>
        public TPrediction Predict<TPrediction>(IVowpalWabbitPredictionFactory<TPrediction> predictionFactory, VowpalWabbit vw = null)
        {
            return this.PredictInternal(predictionFactory, vw ?? this.vw);
        }

        /// <summary>
        /// Learns from this example.
        /// </summary>
        /// <param name="vw">Use this VW instance for learning instead of the one the example was created from.</param>
        protected abstract void LearnInternal(VowpalWabbit vw);

        /// <summary>
        /// Predicts for this example.
        /// </summary>
        /// <param name="vw">Use this VW instance for prediction instead of the one the example was created from.</param>
        protected abstract void PredictInternal(VowpalWabbit vw);

        /// <summary>
        /// Learn from this example and returns the current prediction for it.
        /// </summary>
        /// <typeparam name="TPrediction">The prediction type.</typeparam>
        /// <param name="predictionFactory">The prediction factory to be used. See <see cref="VowpalWabbitPredictionType"/>.</param>
        /// <returns>The prediction for the this example.</returns>
        /// <param name="vw">Use this VW instance for learning instead of the one the example was created from.</param>
        protected abstract TPrediction LearnInternal<TPrediction>(IVowpalWabbitPredictionFactory<TPrediction> predictionFactory, VowpalWabbit vw);

        /// <summary>
        /// Predicts for this example and returns the current prediction for it.
        /// </summary>
        /// <typeparam name="TPrediction">The prediction type.</typeparam>
        /// <param name="predictionFactory">The prediction factory to be used. See <see cref="VowpalWabbitPredictionType"/>.</param>
        /// <returns>The prediction for the this example.</returns>
        /// <param name="vw">Use this VW instance for prediction instead of the one the example was created from.</param>
        protected abstract TPrediction PredictInternal<TPrediction>(IVowpalWabbitPredictionFactory<TPrediction> predictionFactory, VowpalWabbit vw);

        /// <summary>
        /// The optional string version of the example.
        /// </summary>
        public abstract string VowpalWabbitString { get; }

        /// <summary>
        /// The number of feature this example holds.
        /// </summary>
        public abstract ulong NumberOfFeatures { get; }

        /// <summary>
        /// All labels this example holds.
        /// </summary>
        public abstract IEnumerable<ILabel> Labels { get; }

        /// <summary>
        /// Performs application-defined tasks associated with freeing, releasing, or resetting unmanaged resources.
        /// </summary>
        public abstract void Dispose();
    }
}
