using System;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using System.Globalization;
using System.Linq;
using System.Linq.Expressions;
using System.Reflection;
using System.Reflection.Emit;
using System.Security;
using System.Security.Permissions;
using System.Text;
using System.Threading.Tasks;
using Microsoft.Research.MachineLearning.Serializer.Attributes;
using Microsoft.Research.MachineLearning.Serializer.Interfaces;
using Microsoft.Research.MachineLearning.Serializer.Intermediate;
using Microsoft.Research.MachineLearning.Serializer.Reflection;
using Microsoft.Research.MachineLearning.Serializer.Visitors;

namespace Microsoft.Research.MachineLearning.Serializer
{
    public sealed class VowpalWabbitSerializer<TExample, TExampleResult> : IDisposable
    {
        private readonly Func<TExample, TExampleResult> serializer;
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
                // Free managed resources
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
