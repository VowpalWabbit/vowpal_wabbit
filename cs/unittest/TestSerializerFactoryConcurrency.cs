using System.Linq;
using System.Threading.Tasks;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using VW;
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

        public class ConcurrentConstructionExample
        {
            [Feature]
            public float Value { get; set; }
        }
    }
}
