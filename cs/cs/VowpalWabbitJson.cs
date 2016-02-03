// --------------------------------------------------------------------------------------------------------------------
// <copyright file="VowpalWabbitJson.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using Newtonsoft.Json;
using System;
using System.Diagnostics.Contracts;
using VW.Interfaces;
using VW.Serializer;

namespace VW
{
    /// <summary>
    /// A VowpalWabbit wrapper reading from JSON.
    /// </summary>
    public sealed class VowpalWabbitJson : IDisposable
    {
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
        /// <param name="label">
        /// Optional label, taking precedence over "_label" property found in <paramref name="json"/>.
        /// If null, <paramref name="json"/> will be inspected and the "_label" property used as label.
        /// </param>
        public void Learn(string json, ILabel label = null)
        {
            using (var serializer = new VowpalWabbitJsonSerializer(vw))
            using (var example = serializer.ParseAndCreateExample(json, label))
            {
                this.vw.Learn(example);
            }
        }

        /// <summary>
        /// Learns from the given example.
        /// </summary>
        /// <param name="reader">The example to learn.</param>
        /// <param name="label">
        /// Optional label, taking precedence over "_label" property found in <paramref name="reader"/>.
        /// If null, <paramref name="reader"/> will be inspected and the "_label" property used as label.
        /// </param>
        public void Learn(JsonReader reader, ILabel label = null)
        {
            using (var serializer = new VowpalWabbitJsonSerializer(vw))
            using (var example = serializer.ParseAndCreateExample(reader, label))
            {
                this.vw.Learn(example);
            }
        }

        /// <summary>
        /// Learn from the given example and return the current prediction for it.
        /// </summary>
        /// <typeparam name="TPrediction">The prediction type.</typeparam>
        /// <param name="json">The example to learn.</param>
        /// <param name="predictionFactory">The prediction factory to be used. See <see cref="VowpalWabbitPredictionType"/>.</param>
        /// <param name="label">
        /// Optional label, taking precedence over "_label" property found in <paramref name="json"/>.
        /// If null, <paramref name="json"/> will be inspected and the "_label" property used as label.
        /// </param>
        /// <returns>The prediction for the given <paramref name="json"/>.</returns>
        public TPrediction Learn<TPrediction>(string json, IVowpalWabbitPredictionFactory<TPrediction> predictionFactory, ILabel label = null)
        {
            using (var serializer = new VowpalWabbitJsonSerializer(vw))
            using (var example = serializer.ParseAndCreateExample(json, label))
            {
                return this.vw.Learn(example, predictionFactory);
            }
        }

        /// <summary>
        /// Learn from the given example and return the current prediction for it.
        /// </summary>
        /// <typeparam name="TPrediction">The prediction type.</typeparam>
        /// <param name="reader">The example to learn.</param>
        /// <param name="predictionFactory">The prediction factory to be used. See <see cref="VowpalWabbitPredictionType"/>.</param>
        /// <param name="label">
        /// Optional label, taking precedence over "_label" property found in <paramref name="reader"/>.
        /// If null, <paramref name="reader"/> will be inspected and the "_label" property used as label.
        /// </param>
        /// <returns>The prediction for the given <paramref name="reader"/>.</returns>
        public TPrediction Learn<TPrediction>(JsonReader reader, IVowpalWabbitPredictionFactory<TPrediction> predictionFactory, ILabel label = null)
        {
            using (var serializer = new VowpalWabbitJsonSerializer(vw))
            using (var example = serializer.ParseAndCreateExample(reader, label))
            {
                return this.vw.Learn(example, predictionFactory);
            }
        }

        /// <summary>
        /// Predicts for the given example.
        /// </summary>
        /// <param name="json">The example to predict for.</param>
        /// <param name="label">
        /// Optional label, taking precedence over "_label" property found in <paramref name="json"/>.
        /// If null, <paramref name="json"/> will be inspected and the "_label" property used as label.
        /// </param>

        public void Predict(string json, ILabel label = null)
        {
            using (var serializer = new VowpalWabbitJsonSerializer(vw))
            using (var example = serializer.ParseAndCreateExample(json, label))
            {
                this.vw.Predict(example);
            }
        }

        /// <summary>
        /// Predicts for the given example.
        /// </summary>
        /// <param name="reader">The example to predict for.</param>
        /// <param name="label">
        /// Optional label, taking precedence over "_label" property found in <paramref name="reader"/>.
        /// If null, <paramref name="reader"/> will be inspected and the "_label" property used as label.
        /// </param>
        public void Predict(JsonReader reader, ILabel label = null)
        {
            using (var serializer = new VowpalWabbitJsonSerializer(vw))
            using (var example = serializer.ParseAndCreateExample(reader, label))
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
        /// <param name="label">
        /// Optional label, taking precedence over "_label" property found in <paramref name="json"/>.
        /// If null, <paramref name="json"/> will be inspected and the "_label" property used as label.
        /// </param>
        public TPrediction Predict<TPrediction>(string json, IVowpalWabbitPredictionFactory<TPrediction> predictionFactory, ILabel label = null)
        {
            using (var serializer = new VowpalWabbitJsonSerializer(vw))
            using (var example = serializer.ParseAndCreateExample(json, label))
            {
                return this.vw.Predict(example, predictionFactory);
            }
        }

        /// <summary>
        /// Predicts for the given example.
        /// </summary>
        /// <typeparam name="TPrediction">The prediction type.</typeparam>
        /// <param name="reader">The example to predict for.</param>
        /// <param name="predictionFactory">The prediction factory to be used. See <see cref="VowpalWabbitPredictionType"/>.</param>
        /// <param name="label">
        /// Optional label, taking precedence over "_label" property found in <paramref name="reader"/>.
        /// If null, <paramref name="reader"/> will be inspected and the "_label" property used as label.
        /// </param>
        public TPrediction Predict<TPrediction>(JsonReader reader, IVowpalWabbitPredictionFactory<TPrediction> predictionFactory, ILabel label = null)
        {
            using (var serializer = new VowpalWabbitJsonSerializer(vw))
            using (var example = serializer.ParseAndCreateExample(reader, label))
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
