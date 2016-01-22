// --------------------------------------------------------------------------------------------------------------------
// <copyright file="VowpalWabbitJson.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System;
using System.Diagnostics.Contracts;
using VW.Serializer;

namespace VW
{
    /// <summary>
    /// A VowpalWabbit wrapper reading from JSON.
    /// </summary>
    public sealed class VowpalWabbitJson : IDisposable
    {
        private readonly VowpalWabbitJsonSerializer serializer;
        private VowpalWabbit vw;

        /// <summary>
        /// Initializes a new instance of the <see cref="VowpalWabbitJson"/> class.
        /// </summary>
        /// <param name="args">Command line arguments passed to native instance.</param>
        public VowpalWabbitJson(String args) : this(new VowpalWabbit(args))
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="VowpalWabbitJson"/> class.
        /// </summary>
        /// <param name="settings">Arguments passed to native instance.</param>
        public VowpalWabbitJson(VowpalWabbitSettings settings)
            : this(new VowpalWabbit(settings))
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="VowpalWabbitJson"/> class.
        /// </summary>
        /// <param name="vw">The native instance to wrap.</param>
        /// <remarks>This instance takes ownership of <paramref name="vw"/> instance and disposes it.</remarks>
        public VowpalWabbitJson(VowpalWabbit vw)
        {
            if (vw == null)
            {
                throw new ArgumentNullException("vw");
            }
            Contract.EndContractBlock();

            this.vw = vw;
            this.serializer = new VowpalWabbitJsonSerializer(vw);
        }

        /// <summary>
        /// The wrapped VW instance.
        /// </summary>
        public VowpalWabbit Native
        {
            get
            {
                return this.vw;
            }
        }

        /// <summary>
        /// Learns from the given example.
        /// </summary>
        /// <param name="json">The example to learn.</param>
        public void Learn(string json)
        {
            using (var example = this.serializer.Parse(json))
            {
                this.vw.Learn(example);
            }

            // expect:
            // { "_label": { ... }, "ns1": { "feature1": 1, ... }, "ns2": { "feature2":"...}, "_aux": { ... } }
            // Label:
            // Simple: { "Label":1, "Weight":2, "Initial": 5 }
            // ContextualBanditLabel: { "Action": 1, "Cost": 2, "Probability": 3 }
            //
            // Multiline: { "_label": { ... }, "_shared":{ ... }, "_actions":[ { /* example */ }, ],
            // "_action:":5 }
        }

        /// <summary>
        /// Learn from the given example and return the current prediction for it.
        /// </summary>
        /// <typeparam name="TPrediction">The prediction type.</typeparam>
        /// <param name="json">The example to learn.</param>
        /// <param name="label">The label for this <paramref name="example"/>.</param>
        /// <param name="predictionFactory">The prediction factory to be used. See <see cref="VowpalWabbitPredictionType"/>.</param>
        /// <returns>The prediction for the given <paramref name="example"/>.</returns>
        public TPrediction Learn<TPrediction>(string json, IVowpalWabbitPredictionFactory<TPrediction> predictionFactory)
        {
            using (var example = this.serializer.Parse(json))
            {
                return this.vw.Learn(example, predictionFactory);
            }
        }

        /// <summary>
        /// Predicts for the given example.
        /// </summary>
        /// <param name="json">The example to predict for.</param>
        public void Predict(string json)
        {
            using (var example = this.serializer.Parse(json))
            {
                this.vw.Predict(example);
            }
        }

        /// <summary>
        /// Predicts for the given example.
        /// </summary>
        /// <typeparam name="TPrediction">The prediction type.</typeparam>
        /// <param name="json">The example to predict for.</param>
        /// <param name="predictionFactory">The prediction factory to be used. See <see cref="VowpalWabbitPredictionType"/>.</param>
        public TPrediction Predict<TPrediction>(string json, IVowpalWabbitPredictionFactory<TPrediction> predictionFactory)
        {
            using (var example = this.serializer.Parse(json))
            {
                return this.vw.Predict(example, predictionFactory);
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
                if (this.vw != null)
                {
                    this.vw.Dispose();
                    this.vw = null;
                }
            }
        }
    }
}
