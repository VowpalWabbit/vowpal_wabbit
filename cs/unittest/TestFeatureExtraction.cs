using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using VW;
using VW.Serializer;
using VW.Serializer.Attributes;

namespace cs_unittest
{
    [TestClass]
    public class TestFeatureExtractionClass
    {
        public class Features
        {
            [Feature]
            public float F1 { get; set; }

            [Feature(FeatureGroup = 'l')]
            public string Location { get; set; }
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit")]
        public void TestFeatureExtraction()
        {
            using (var vw = new VowpalWabbit<Features>("--noconstant"))
            using (var serializer = vw.Serializer.Create(vw.Native))
            using (var example = serializer.Serialize(new Features { F1 = 3.2f, Location = "New York" }))
            {
                var singleExample = example as VowpalWabbitSingleLineExampleCollection;
                Assert.IsNotNull(singleExample);

                foreach (var ns in singleExample.Example)
                {
                    Console.WriteLine(ns.Index);

                    foreach (var feature in ns)
                    {
                        Console.WriteLine("{0}:{1}", feature.FeatureIndex, feature.X);
                    }
                }

                var namespaces = singleExample.Example.ToArray();

                Assert.AreEqual((byte)' ', namespaces[0].Index);
                CollectionAssert.AreEqual(
                    new[] {
                        new VowpalWabbitFeature(singleExample.Example, 3.2f, 610696),
                    },
                    namespaces[0].ToArray());

                Assert.AreEqual((byte)'l', namespaces[1].Index);
                CollectionAssert.AreEqual(
                    new[] {
                        new VowpalWabbitFeature(singleExample.Example, 1, 414696),
                        new VowpalWabbitFeature(singleExample.Example, 1, 380324),
                    },
                    namespaces[1].ToArray());
            }
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit")]
        public void TestJsonFeatureExtraction()
        {
            string json = "{\"ns1\":{\"location\":\"New York\", \"f2\":3.4}}";

            using (var vw = new VowpalWabbit("-b 3 --noconstant"))
            using (var serializer = new VowpalWabbitJsonSerializer(vw))
            using (var result = serializer.ParseAndCreate(json))
            {
                var singleExample = result as VowpalWabbitSingleLineExampleCollection;
                Assert.IsNotNull(singleExample);
                if (singleExample != null)
                {
                    foreach (var ns in singleExample.Example)
                    {
                        Console.WriteLine(ns.Index);

                        foreach (var feature in ns)
                        {
                            Console.WriteLine("{0}:{1}", feature.FeatureIndex, feature.X);
                        }
                    }

                    var ns1 = singleExample.Example.ToArray();
                    Assert.AreEqual(1, ns1.Length);
                    Assert.AreEqual((byte)'n', ns1[0].Index);
                    CollectionAssert.AreEqual(
                            new[] {
                                new VowpalWabbitFeature(singleExample.Example, 1, 12),
                                new VowpalWabbitFeature(singleExample.Example, 3.4f, 28)
                            },
                            ns1[0].ToArray());
                }

                // for documentation purpose only
                var multiExample = result as VowpalWabbitMultiLineExampleCollection;
                Assert.IsNull(multiExample);
                if (multiExample != null)
                {
                    foreach (var example in multiExample.Examples)
                    {
                        foreach (var ns in example)
                        {
                            Console.WriteLine(ns.Index);

                            foreach (var feature in ns)
                            {
                                Console.WriteLine("{0}:{1}", feature.FeatureIndex, feature.X);
                            }
                        }                        
                    }
                }
            }
        }
    }
}
