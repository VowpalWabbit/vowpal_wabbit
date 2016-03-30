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
        public void TestFeatureExtraction()
        {
            using (var vw = new VowpalWabbit<Features>("--noconstant"))
            using (var serializer = vw.Serializer.Create(vw.Native))
            using (var example = serializer.Serialize(new Features { F1 = 3.2f, Location = "New York" }))
            {
                var singleExample = example as VowpalWabbitSingleLineExampleCollection;
                Assert.IsNotNull(singleExample);

                var namespaces = singleExample.Example.ToArray();

                Assert.AreEqual((byte)' ', namespaces[0].Index);
                CollectionAssert.AreEqual(
                    new[] {
                        new VowpalWabbitFeature(3.2f, 610696),
                    },
                    namespaces[0].ToArray());

                Assert.AreEqual((byte)'l', namespaces[1].Index);
                CollectionAssert.AreEqual(
                    new[] {
                        new VowpalWabbitFeature(1, 414696),
                        new VowpalWabbitFeature(1, 380324),
                    },
                    namespaces[1].ToArray());
            }
        }
    }
}
