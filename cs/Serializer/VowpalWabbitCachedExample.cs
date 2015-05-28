// --------------------------------------------------------------------------------------------------------------------
// <copyright file="VowpalWabbitCachedExample.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Microsoft.Research.MachineLearning.Serializer
{
    /// <summary>
    /// A proxy to intercept IDisposable calls to allow to safely cache the example.
    /// </summary>
    /// <typeparam name="TExample">The underlying example type to cache.</typeparam>
    internal sealed class VowpalWabbitCachedExample<TExample> : IVowpalWabbitExample
    {
        private readonly VowpalWabbitSerializer<TExample> serializer;

        internal VowpalWabbitCachedExample(VowpalWabbitSerializer<TExample> serializer, IVowpalWabbitExample example)
        {
            this.serializer = serializer;
            this.UnderlyingExample = example;
            this.LastRecentUse = DateTime.Now;
        }

        /// <summary>
        /// The underlying examle that is proxied too.
        /// </summary>
        internal IVowpalWabbitExample UnderlyingExample { get; private set; }

        /// <summary>
        /// The last time this item was retrieved from the cache
        /// </summary>
        internal DateTime LastRecentUse { get; set; }

        public float CostSensitivePrediction
        {
            get { return this.UnderlyingExample.CostSensitivePrediction; }
        }

        public string Diff(IVowpalWabbitExample other, bool sameOrder)
        {
            return this.UnderlyingExample.Diff(other, sameOrder);
        }

        public bool IsNewLine
        {
            get { return this.UnderlyingExample.IsNewLine; }
        }

        public float Learn()
        {
            return this.UnderlyingExample.Learn();
        }

        public int[] MultilabelPredictions
        {
            get { return this.UnderlyingExample.MultilabelPredictions; }
        }

        public float[] TopicPredictions
        {
            get { return this.UnderlyingExample.TopicPredictions; }
        }

        public float Predict()
        {
            return this.UnderlyingExample.Predict();
        }

        public void Finish()
        {
            this.UnderlyingExample.Finish();
        }

        public void Dispose()
        {
            // return example to cache.
            this.serializer.ReturnExampleToCache(this);
        }
    }
}
