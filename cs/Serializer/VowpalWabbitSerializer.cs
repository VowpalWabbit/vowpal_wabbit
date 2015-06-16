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
using MoreLinq;
using Microsoft.Research.MachineLearning.Serializer.Attributes;

namespace Microsoft.Research.MachineLearning.Serializer
{
    /// <summary>
    /// A serializer from a user type (TExample) to a native Vowpal Wabbit example type.
    /// </summary>
    /// <typeparam name="TExample">The source example type.</typeparam>
    public sealed class VowpalWabbitSerializer<TExample> : IDisposable
    {
        private readonly VowpalWabbitSerializerSettings settings;

        private readonly Func<TExample, IVowpalWabbitExample> serializer;

        private Dictionary<TExample, VowpalWabbitCachedExample<TExample>> exampleCache;

        internal VowpalWabbitSerializer(Func<TExample, IVowpalWabbitExample> serializer, VowpalWabbitSerializerSettings settings)
        {
            this.serializer = serializer;
            this.settings = settings ?? new VowpalWabbitSerializerSettings();

            var cacheableAttribute = (CacheableAttribute) typeof (TExample).GetCustomAttributes(typeof (CacheableAttribute), true).FirstOrDefault();
            if (cacheableAttribute == null)
            {
                return;
            }

            if (this.settings.EnableExampleCaching)
            {
                if (cacheableAttribute.EqualityComparer == null)
                {
                    this.exampleCache = new Dictionary<TExample, VowpalWabbitCachedExample<TExample>>();
                }
                else
                {
                    if (!typeof(IEqualityComparer<TExample>).IsAssignableFrom(cacheableAttribute.EqualityComparer))
                    {
                        throw new ArgumentException(
                            string.Format(
                                CultureInfo.InvariantCulture,
                                "EqualityComparer ({1}) specified in [Cachable] of {0} must implement IEqualityComparer<{0}>",
                                typeof(TExample),
                                cacheableAttribute.EqualityComparer));
                    }

                    var comparer = (IEqualityComparer<TExample>)Activator.CreateInstance(cacheableAttribute.EqualityComparer);
                    this.exampleCache = new Dictionary<TExample, VowpalWabbitCachedExample<TExample>>(comparer);
                }
            }
        }

        /// <summary>
        /// Serialize the example.
        /// </summary>
        /// <param name="example">The example to serialize.</param>
        /// <returns>The serialized example.</returns>
        /// <remarks>If TExample is annotated using the Cachable attribute, examples are returned from cache.</remarks>
        public IVowpalWabbitExample Serialize(TExample example)
        {
            if (this.exampleCache == null)
            {
                return this.serializer(example);
            }

            VowpalWabbitCachedExample<TExample> result;
            if (this.exampleCache.TryGetValue(example, out result))
            {
                result.LastRecentUse = DateTime.Now;
            }
            else
            {
                result = new VowpalWabbitCachedExample<TExample>(this, this.serializer(example));
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
                if (this.exampleCache != null)
                {
                    foreach (var example in this.exampleCache.Values)
                    {
                        example.UnderlyingExample.Dispose();
                    }

                    this.exampleCache = null;
                }
            }
        }

        internal void ReturnExampleToCache(VowpalWabbitCachedExample<TExample> example)
        {
            if (this.exampleCache == null)
            {
                throw new ObjectDisposedException("VowpalWabbitSerializer");
            }

            // if we reach the cache boundary, dispose the oldest example
            if (this.exampleCache.Count > this.settings.MaxExampleCacheSize)
            {
                var minElement = this.exampleCache.MinBy(kv => kv.Value.LastRecentUse);
                
                this.exampleCache.Remove(minElement.Key);
                minElement.Value.UnderlyingExample.Dispose();
            }
        }
    }
}
