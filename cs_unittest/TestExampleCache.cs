using System;
using System.Collections.Generic;
using System.IO;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using VW;
using VW.Interfaces;
using VW.Labels;
using VW.Serializer;
using VW.Serializer.Attributes;

namespace cs_unittest
{
    [TestClass]
    public class TestExampleCacheCases : TestBase
    {
#if DEBUG
        [TestMethod]
        [ExpectedException(typeof(NotSupportedException))]
        public void TestExampleCacheForLearning()
        {
            using (var vw = new VowpalWabbit<CachedData>(new VowpalWabbitSettings(string.Empty, enableExampleCaching: true)))
            {
                vw.Learn(new CachedData(), new SimpleLabel());
            }
        }
#else
        [TestMethod]
        [ExpectedException(typeof(NullReferenceException))]
        public void TestExampleCacheForLearning()
        {
            using (var vw = new VowpalWabbit<CachedData>(new VowpalWabbitSettings(string.Empty, enableExampleCaching: true)))
            {
                vw.Learn(new CachedData(), new SimpleLabel());
            }
        }
#endif

        [TestMethod]
        public void TestExampleCacheDisabledForLearning()
        {
            using (var vw = new VowpalWabbit<CachedData>(new VowpalWabbitSettings(enableExampleCaching: false)))
            {
                vw.Learn(new CachedData(), new SimpleLabel());

            }
        }

        [TestMethod]
        public void TestExampleCache()
        {
            var random = new Random(123);
            var examples = new List<CachedData>();

            for (int i = 0; i < 1000; i++)
            {
                examples.Add(new CachedData
                {
                    Label = new SimpleLabel { Label = 1 },
                    Feature = random.NextDouble()
                });
            }

            for (int i = 0; i < 1000; i++)
            {
                var cachedData = new CachedData
                {
                    Label = new SimpleLabel { Label = 2 },
                    Feature = 10 + random.NextDouble()
                };

                examples.Add(cachedData);
                examples.Add(cachedData);
            }

            using (var vw = new VowpalWabbit<CachedData>(new VowpalWabbitSettings("-k -c --passes 10", enableExampleCaching: false)))
            {
                foreach (var example in examples)
                {
                    vw.Learn(example, example.Label);
                }

                vw.Native.RunMultiPass();
                vw.Native.SaveModel("model1");
            }

            using (var vwModel = new VowpalWabbitModel(new VowpalWabbitSettings("-t", modelStream: File.OpenRead("model1"))))
            using (var vwCached = new VowpalWabbit<CachedData>(new VowpalWabbitSettings(model: vwModel, enableExampleCaching: true, maxExampleCacheSize: 5 )))
            using (var vw = new VowpalWabbit<CachedData>(new VowpalWabbitSettings(model: vwModel, enableExampleCaching: false )))
            {
                foreach (var example in examples)
                {
                    var cachedPrediction = vwCached.Predict(example, VowpalWabbitPredictionType.Scalar);
                    var prediction = vw.Predict(example, VowpalWabbitPredictionType.Scalar);

                    Assert.AreEqual(prediction, cachedPrediction);
                    Assert.AreEqual(example.Label.Label, Math.Round(prediction));
                }
            }
        }
    }
     
    [Cacheable]
    public class CachedData 
    {
        [Feature]
        public double Feature { get; set; }

        public SimpleLabel Label
        {
            get;
            set;
        }
    }
}
