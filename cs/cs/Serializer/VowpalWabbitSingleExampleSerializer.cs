// --------------------------------------------------------------------------------------------------------------------
// <copyright file="VowpalWabbitSerializer.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Diagnostics.Contracts;
using System.Globalization;
using System.Linq;
using VW.Labels;
using VW.Serializer.Attributes;

namespace VW.Serializer
{
    /// <summary>
    /// A serializer from a user type (TExample) to a native Vowpal Wabbit example type.
    /// </summary>
    /// <typeparam name="TExample">The source example type.</typeparam>
    public sealed class VowpalWabbitSingleExampleSerializer<TExample> : IVowpalWabbitSerializer<TExample>, IVowpalWabbitExamplePool
    {
        private class CacheEntry
        {
            internal VowpalWabbitExample Example;

            internal DateTime LastRecentUse;

#if DEBUG
            internal bool InUse;
#endif
        }

        private readonly VowpalWabbitSingleExampleSerializerCompiler<TExample> compiler;

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

        private readonly VowpalWabbit vw;

        private readonly Action<VowpalWabbitMarshalContext, TExample, ILabel> serializerFunc;

        internal VowpalWabbitSingleExampleSerializer(VowpalWabbitSingleExampleSerializerCompiler<TExample> compiler, VowpalWabbit vw)
        {
            if (compiler == null)
                throw new ArgumentNullException("compiler");
            Contract.Ensures(vw != null);
            Contract.EndContractBlock();

            this.vw = vw;
            this.compiler = compiler;

            var exampleType = typeof(TExample);
            if (!exampleType.IsVisible)
                throw new ArgumentException($"Type '{typeof(TExample)}' must be public and all enclosing types must be public.");

            this.serializerFunc = compiler.Func(vw);

            var cacheableAttribute = (CacheableAttribute) typeof (TExample).GetCustomAttributes(typeof (CacheableAttribute), true).FirstOrDefault();
            if (cacheableAttribute == null)
                return;

            if (this.vw.Settings.EnableExampleCaching)
            {
                if (cacheableAttribute.EqualityComparer == null)
                    this.exampleCache = new Dictionary<TExample, CacheEntry>();
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
        /// True if this instance caches examples, false otherwise.
        /// </summary>
        public bool CachesExamples
        {
            get { return this.exampleCache != null; }
        }

        /// <summary>
        /// True if string examples are generated in parallel to native examples.
        /// </summary>
        public bool EnableStringExampleGeneration
        {
            get { return !this.compiler.DisableStringExampleGeneration; }
        }

        /// <summary>
        /// Serializes the given <paramref name="example"/> to VW string format.
        /// </summary>
        /// <param name="example">The example to serialize.</param>
        /// <param name="label">The label to serialize.</param>
        /// <param name="index">The optional index of the example, the <paramref name="label"/> should be attributed to.</param>
        /// <param name="dictionary">Dictionary used for dictify operation.</param>
        /// <param name="fastDictionary">Dictionary used for dictify operation.</param>
        /// <returns>The resulting VW string.</returns>
        public string SerializeToString(TExample example, ILabel label = null, int? index = null, Dictionary<string, string> dictionary = null, Dictionary<object, string> fastDictionary = null)
        {
            Contract.Requires(example != null);

            using (var context = new VowpalWabbitMarshalContext(vw, dictionary, fastDictionary))
            {
                this.serializerFunc(context, example, label);
                return context.ToString();
            }
        }

        /// <summary>
        /// Serialize the example.
        /// </summary>
        /// <param name="example">The example to serialize.</param>
        /// <param name="label">The label to be serialized.</param>
        /// <param name="index">The optional index of the example, the <paramref name="label"/> should be attributed to.</param>
        /// <returns>The serialized example.</returns>
        /// <remarks>If <typeparamref name="TExample"/> is annotated using the Cachable attribute, examples are returned from cache.</remarks>
        VowpalWabbitExampleCollection IVowpalWabbitSerializer<TExample>.Serialize(TExample example, ILabel label, int? index)
        {
            // dispatch
            return new VowpalWabbitSingleLineExampleCollection(vw, Serialize(example, label, index));
        }

        /// <summary>
        /// Serialize the example.
        /// </summary>
        /// <param name="example">The example to serialize.</param>
        /// <param name="label">The label to be serialized.</param>
        /// <param name="index">The optional index of the example, the <paramref name="label"/> should be attributed to.</param>
        /// <returns>The serialized example.</returns>
        /// <remarks>If <typeparamref name="TExample"/> is annotated using the Cachable attribute, examples are returned from cache.</remarks>
        public VowpalWabbitExample Serialize(TExample example, ILabel label = null, int? index = null)
        {
            Contract.Requires(example != null);
            Contract.Requires(index == null);

            if (this.exampleCache == null || label != null)
            {
                using (var context = new VowpalWabbitMarshalContext(vw))
                {
                    this.serializerFunc(context, example, label);

                    var vwExample = context.ExampleBuilder.CreateExample();

                    if (this.EnableStringExampleGeneration)
                        vwExample.VowpalWabbitString = context.ToString();

                    return vwExample;
                }
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
                VowpalWabbitExample nativeExample = null;

                try
                {
                    using (var context = new VowpalWabbitMarshalContext(this))
                    {
                        this.serializerFunc(context, example, label);
                        nativeExample = context.ExampleBuilder.CreateExample();
                    }

                    result = new CacheEntry
                    {
                        Example = nativeExample,
                        LastRecentUse = DateTime.UtcNow
                    };

                    this.exampleCache.Add(example, result);

#if DEBUG
                    this.reverseLookup.Add(result.Example, result);
#endif
                }
                catch(Exception e)
                {
                    if (nativeExample != null)
                        nativeExample.Dispose();

                    throw e;
                }
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
        /// The associated native instance.
        /// </summary>
        public VowpalWabbit Native
        {
            get
            {
                return this.vw;
            }
        }

        /// <summary>
        /// Gets an already allocated instance from the example pool or creates a new one.
        /// </summary>
        /// <returns></returns>
        public VowpalWabbitExample GetOrCreateNativeExample()
        {
            return new VowpalWabbitExample(owner: this, example: this.vw.GetOrCreateNativeExample());
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
            if (this.exampleCache.Count > this.vw.Settings.MaxExampleCacheSize)
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
