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
        private IVowpalWabbitExample example;

        internal VowpalWabbitCachedExample(VowpalWabbitSerializer<TExample> serializer, IVowpalWabbitExample example)
        {
            this.serializer = serializer;
            this.example = example;
            this.LastRecentUse = DateTime.Now;
        }

        /// <summary>
        /// The underlying examle that is proxied too.
        /// </summary>
        public VowpalWabbitExample UnderlyingExample
        {
            get
            {
                return this.example.UnderlyingExample;
            }
        }

        /// <summary>
        /// The last time this item was retrieved from the cache
        /// </summary>
        internal DateTime LastRecentUse { get; set; }

        public void Dispose()
        {
            // return example to cache.
            this.serializer.ReturnExampleToCache(this);
        }


        void IDisposable.Dispose()
        {
            throw new NotImplementedException();
        }

        TPrediction IVowpalWabbitExample.Learn<TPrediction>()
        {
            throw new NotSupportedException("As labels cannot be updated once the example is created, cached examples cannot be used for learning");
            // return this.example.Learn<TPrediction>();
        }

        TPrediction IVowpalWabbitExample.Predict<TPrediction>()
        {
            return this.example.Predict<TPrediction>();
        }
    }
}
