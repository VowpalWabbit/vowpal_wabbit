// --------------------------------------------------------------------------------------------------------------------
// <copyright file="VowpalWabbitSerializer.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using Microsoft.Research.MachineLearning.Serializer.Attributes;

namespace Microsoft.Research.MachineLearning.Serializer
{
    public sealed class VowpalWabbitSerializer<TExample, TExampleResult> : IDisposable
    {
        private readonly Func<TExample, TExampleResult> serializer;

        // TODO: replace with MemoryCache
        private Dictionary<TExample, TExampleResult> exampleCache;

        internal VowpalWabbitSerializer(Func<TExample, TExampleResult> serializer)
        {
            this.serializer = serializer;

            var cacheableAttribute = (CacheableAttribute) typeof (TExample).GetCustomAttributes(typeof (CacheableAttribute), true).FirstOrDefault();
            if (cacheableAttribute == null)
            {
                return;
            }

            if (cacheableAttribute.EqualityComparer == null)
            {
                this.exampleCache = new Dictionary<TExample, TExampleResult>();
            }
            else
            {
                if (!typeof (IEqualityComparer<TExample>).IsAssignableFrom(cacheableAttribute.EqualityComparer))
                {
                    throw new ArgumentException(
                        string.Format(
                            CultureInfo.InvariantCulture,
                            "EqualityComparer ({1}) specified in [Cachable] of {0} must implement IEqualityComparer<{0}>",
                            typeof (TExample),
                            cacheableAttribute.EqualityComparer));
                }

                var comparer = (IEqualityComparer<TExample>) Activator.CreateInstance(cacheableAttribute.EqualityComparer);
                this.exampleCache = new Dictionary<TExample, TExampleResult>(comparer);
            }
        }

        /// <summary>
        /// Serialize the example.
        /// </summary>
        /// <param name="example">The example to serialize.</param>
        /// <returns>The serialized example.</returns>
        /// <remarks>If TExample is annotated using the Cachable attribute, examples are returned from cache.</remarks>
        public TExampleResult Serialize(TExample example)
        {
            if (this.exampleCache == null)
            {
                return this.serializer(example);
            }

            TExampleResult result;
            if (!this.exampleCache.TryGetValue(example, out result))
            {
                result = this.serializer(example);
                this.exampleCache.Add(example, result);
            }

            return result;
        }

        public void Dispose()
        {
            this.Dispose(true);
            GC.SuppressFinalize(this);
        }

        private void Dispose(bool disposing)
        {
            if (disposing)
            {
                var disposableDictionary = this.exampleCache as IDictionary<object, IDisposable>;
                if (disposableDictionary != null)
                {
                    foreach (var value in disposableDictionary.Values)
                    {
                        value.Dispose();
                    }

                    this.exampleCache = null;
                }
            }
        }
    }
}
