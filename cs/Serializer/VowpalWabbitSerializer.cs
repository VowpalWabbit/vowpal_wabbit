// --------------------------------------------------------------------------------------------------------------------
// <copyright file="VowpalWabbitSerializer.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics.Contracts;
using System.Globalization;
using System.Linq;
using System.Linq.Expressions;
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

#if DEBUG
            internal bool InUse;
#endif
        }

        private readonly VowpalWabbitSettings settings;

        private readonly Func<VowpalWabbit, TExample, ILabel, VowpalWabbitExample> serializer;

        private Dictionary<TExample, CacheEntry> exampleCache;

#if DEBUG
        /// <summary>
        /// Reverse lookup from native example to cache entry to enable proper usage.
        /// </summary>
        /// <remarks>
        /// To avoid any performance impact this is only enabled in debug mode.
        /// </remarks>
        private readonly Dictionary<VowpalWabbitExample, CacheEntry> reverseLookup;
#endif

        public VowpalWabbitSerializerForVWInstance CreateSerializer(VowpalWabbit vw)
        {
            // TODO: run 2nd lambda that creates the instance that holds all the meta data once (essentially doing all the hash once)
            // TODO: create expression of Func<VW, Func<TExample, ILabel, VowpalWabbitExample>>
            // the first func is the closure for static method data!!! a bit sick already. 
        }

        internal VowpalWabbitSerializer(Func<VowpalWabbit, TExample, ILabel, VowpalWabbitExample> serializer, Expression serializerExpression, VowpalWabbitSettings settings)
        {
            if (serializer == null)
            {
                throw new ArgumentNullException("serializer");
            }
            Contract.Ensures(this.settings != null);
            Contract.EndContractBlock();

            this.serializer = serializer;
            this.NativeSerializerExpression = serializerExpression;
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

#if DEBUG
                this.reverseLookup = new Dictionary<VowpalWabbitExample, CacheEntry>(new ReferenceEqualityComparer<VowpalWabbitExample>());
#endif
            }
        }

        /// <summary>
        /// Useful for debugging.
        /// </summary>
        [EditorBrowsable(EditorBrowsableState.Never)]
        public Expression NativeSerializerExpression { get; private set; }

        [EditorBrowsable(EditorBrowsableState.Never)]
        public Expression StringSerializerExpression { get; set; }

        /// <summary>
        /// True if this instance caches examples, false otherwise.
        /// </summary>
        public bool CachesExamples
        {
            get { return this.exampleCache != null; }
        }

        /// <summary>
        /// Serialize the example.
        /// </summary>
        /// <param name="vw">The vw instance.</param>
        /// <param name="example">The example to serialize.</param>
        /// <param name="label">The label to be serialized.</param>
        /// <returns>The serialized example.</returns>
        /// <remarks>If TExample is annotated using the Cachable attribute, examples are returned from cache.</remarks>
        public VowpalWabbitExample Serialize(VowpalWabbit vw, TExample example, ILabel label = null)
        {
            Contract.Requires(vw != null);
            Contract.Requires(example != null);

            if (this.exampleCache == null || label != null)
            {
                return this.serializer(vw, example, label);
            }

            CacheEntry result;
            if (this.exampleCache.TryGetValue(example, out result))
            {
                result.LastRecentUse = DateTime.UtcNow;

#if DEBUG
                if (result.InUse)
                {
                    throw new ArgumentException("Cached example already in use.");
                }
#endif
            }
            else
            {
                result = new CacheEntry 
                {
                    Example =  new VowpalWabbitExample(owner: this, example: this.serializer(vw, example, label)),
                    LastRecentUse = DateTime.UtcNow
                };
                this.exampleCache.Add(example, result);

#if DEBUG
                this.reverseLookup.Add(result.Example, result);
#endif
            }

#if DEBUG
            result.InUse = true;
#endif

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

        /// <summary>
        /// Accepts an example back into this pool.
        /// </summary>
        /// <param name="example">The example to be returned.</param>
		public void ReturnExampleToPool(VowpalWabbitExample example)
        {
            if (this.exampleCache == null)
            {
                throw new ObjectDisposedException("VowpalWabbitSerializer");
            }

#if DEBUG
            CacheEntry cacheEntry;
            if (!this.reverseLookup.TryGetValue(example, out cacheEntry))
            {
                throw new ArgumentException("Example is not found in pool");
            }

            if (!cacheEntry.InUse)
            {
                throw new ArgumentException("Unused example returned");
            }

            cacheEntry.InUse = false;
#endif

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

#if DEBUG
                this.reverseLookup.Remove(min.Value.Example);
#endif

                this.exampleCache.Remove(min.Key);
                min.Value.Example.InnerExample.Dispose();
            }
        }

        private class ReferenceEqualityComparer<T> : IEqualityComparer<T>
        {
            public bool Equals(T x, T y)
            {
                return object.ReferenceEquals(x, y);
            }

            public int GetHashCode(T obj)
            {
                return obj.GetHashCode();
            }
        }
    }
}
