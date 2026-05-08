using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using VW;
using VW.Serializer;
using VW.Serializer.Attributes;

namespace cs_unittest
{
    [TestClass]
    public class TestSerializerFactoryConcurrency : TestBase
    {
        [TestMethod]
        [TestCategory("Vowpal Wabbit")]
        public void TestConcurrentVowpalWabbitConstruction()
        {
            // Regression test for https://github.com/VowpalWabbit/vowpal_wabbit/issues/4919:
            // VowpalWabbitSerializerFactory's static cache was a non-thread-safe Dictionary,
            // so concurrent construction of VowpalWabbit<TExample> with a cold cache could
            // throw InvalidOperationException from Dictionary.TryInsert.
            var tasks = Enumerable.Range(0, 10).Select(_ => Task.Run(() =>
            {
                var settings = new VowpalWabbitSettings();
                using (var vw = new VowpalWabbit<ConcurrentConstructionExample>(settings))
                {
                }
            })).ToArray();

            Task.WaitAll(tasks);
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit")]
        public void TestSerializerCacheHonorsCustomFeaturizerSequenceEquality()
        {
            // The Key.Equals method compares CustomFeaturizer with SequenceEqual, so two
            // distinct list instances with identical content must hit the same cache entry.
            // Previously Key.GetHashCode used the list's reference hash, which violated the
            // Equals/GetHashCode contract and caused spurious cache misses (and the cache
            // would silently store duplicate entries for "equivalent" keys).
            var settings1 = new VowpalWabbitSettings { CustomFeaturizer = new List<Type> { typeof(CustomFeaturizer) } };
            var settings2 = new VowpalWabbitSettings { CustomFeaturizer = new List<Type> { typeof(CustomFeaturizer) } };

            var compiler1 = VowpalWabbitSerializerFactory.CreateSerializer<MyContext>(settings1);
            var compiler2 = VowpalWabbitSerializerFactory.CreateSerializer<MyContext>(settings2);

            Assert.AreSame(compiler1, compiler2);
        }

        public class ConcurrentConstructionExample
        {
            [Feature]
            public float Value { get; set; }
        }
    }
}
