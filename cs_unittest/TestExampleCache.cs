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
        [TestMethod]
        [ExpectedException(typeof(NotSupportedException))]
        public void TestExampleCacheForLearning()
        {
            using (var vw = new VowpalWabbit<CachedData>(string.Empty))
            {
                using (var example = vw.ReadExample(new CachedData()))
                {
                    example.Learn();
                }
            }
        }

        [TestMethod]
        public void TestExampleCacheDisabledForLearning()
        {
            using (var vw = new VowpalWabbit<CachedData>(string.Empty, new VowpalWabbitSettings
                {
                    EnableExampleCaching = false
                }))
            {
                using (var example = vw.ReadExample(new CachedData()))
                {
                    example.Learn();
                }
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

            using (var vw = new VowpalWabbit<CachedData>("-k -c --passes 10", new VowpalWabbitSettings
            {
                EnableExampleCaching = false
            }))
            {
                foreach (var example in examples)
                {
                    using (var vwExample = vw.ReadExample(example))
                    {
                        vwExample.Learn();
                    }
                }

                vw.RunMultiPass();
                vw.SaveModel("model1");
            }

            using (var vwModel = new VowpalWabbitModel("-t", File.OpenRead("model1")))
            using (var vwCached = new VowpalWabbit<CachedData>(vwModel, new VowpalWabbitSettings { EnableExampleCaching = true, MaxExampleCacheSize = 5 }))
            using (var vw = new VowpalWabbit<CachedData>(vwModel, new VowpalWabbitSettings { EnableExampleCaching = false }))
            {
                foreach (var example in examples)
                {
                    using (var vwCachedExample = vwCached.ReadExample(example))
                    using (var vwExample = vw.ReadExample(example))
                    {
                        var cachedPrediction = vwCachedExample.Predict<VowpalWabbitScalarPrediction>();
                        var prediction = vwExample.Predict<VowpalWabbitScalarPrediction>();

                        Assert.AreEqual(prediction.Value, cachedPrediction.Value);
                        Assert.AreEqual(example.Label.Label, Math.Round(prediction.Value));
                    }
                }
            }
        }
    }
     
    [Cacheable]
    public class CachedData : IExample
    {
        [Feature]
        public double Feature { get; set; }

        public SimpleLabel Label
        {
            get;
            set;
        }

        ILabel IExample.Label
        {
            get { return this.Label; }
        }
    }
}
