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
using VW.Interfaces;
using VW.Serializer.Attributes;
using VW.Serializer.Visitors;

namespace VW.Serializer
{
    /// <summary>
    /// A serializer from a user type (TExample) to a native Vowpal Wabbit example type.
    /// </summary>
    /// <typeparam name="TExample">The source example type.</typeparam>
    public sealed class VowpalWabbitSerializer<TExample> : IDisposable, IVowpalWabbitExamplePool
    {
        private class CacheEntry
        {
            internal VowpalWabbitExample Example;

            internal DateTime LastRecentUse;
        }

        private readonly VowpalWabbitSettings settings;

        private readonly Func<VowpalWabbit, TExample, ILabel, VowpalWabbitExample> serializer;

        private Dictionary<TExample, CacheEntry> exampleCache;

        internal VowpalWabbitSerializer(Func<VowpalWabbit, TExample, ILabel, VowpalWabbitExample> serializer, VowpalWabbitSettings settings)
        {
            if (serializer == null)
            {
                throw new ArgumentNullException("serializer");
            }

            this.serializer = serializer;
            this.settings = settings ?? new VowpalWabbitSettings();

            var cacheableAttribute = (CacheableAttribute) typeof (TExample).GetCustomAttributes(typeof (CacheableAttribute), true).FirstOrDefault();
            if (cacheableAttribute == null)
            {
                return;
            }

            if (this.settings.EnableExampleCaching)
            {
                if (cacheableAttribute.EqualityComparer == null)
                {
                    this.exampleCache = new Dictionary<TExample, CacheEntry>();
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
                    this.exampleCache = new Dictionary<TExample, CacheEntry>(comparer);
                }
            }
        }

        public bool CachesExamples
        {
            get { return this.exampleCache != null; }
        }

        /// <summary>
        /// Serialize the example.
        /// </summary>
        /// <param name="example">The example to serialize.</param>
        /// <returns>The serialized example.</returns>
        /// <remarks>If TExample is annotated using the Cachable attribute, examples are returned from cache.</remarks>
        public VowpalWabbitExample Serialize(VowpalWabbit vw, TExample example, ILabel label = null)
        {
            if (this.exampleCache == null || label != null)
            {
                return this.serializer(vw, example, label);
            }

            CacheEntry result;
            if (this.exampleCache.TryGetValue(example, out result))
            {
                result.LastRecentUse = DateTime.UtcNow;
            }
            else
            {
                result = new CacheEntry 
                {
                    Example =  new VowpalWabbitExample(owner: this, example: this.serializer(vw, example, label)),
                    LastRecentUse = DateTime.UtcNow
                };
                this.exampleCache.Add(example, result);
            }

            // TODO: support Label != null here and update cached example using new label
            return result.Example;
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
                if (this.exampleCache != null)
                {
                    foreach (var example in this.exampleCache.Values)
                    {
                        example.Example.InnerExample.Dispose();
                    }

                    this.exampleCache = null;
                }
            }
        }

		public void ReturnExampleToPool(VowpalWabbitExample ex)
        {
            if (this.exampleCache == null)
            {
                throw new ObjectDisposedException("VowpalWabbitSerializer");
            }

            // if we reach the cache boundary, dispose the oldest example
            if (this.exampleCache.Count > this.settings.MaxExampleCacheSize)
            {
                var enumerator = this.exampleCache.GetEnumerator();

                // this.settings.MaxExampleCacheSize is >= 1
                enumerator.MoveNext();

                var min = enumerator.Current;
                while (enumerator.MoveNext())
                {
                    if (min.Value.LastRecentUse > enumerator.Current.Value.LastRecentUse)
                    {
                        min = enumerator.Current;
                    }
                }

                this.exampleCache.Remove(min.Key);
                min.Value.Example.InnerExample.Dispose();
            }
        }
    }
}
