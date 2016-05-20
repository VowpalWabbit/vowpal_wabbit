using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using VW;
using VW.Labels;
using VW.Serializer;

namespace cs_unittest
{
    [TestClass]
    public class TestDynamicClass
    {
        [TestMethod]
        public void TestDynamic()
        {
            using (var vw = new VowpalWabbit("--cb_adf --rank_all"))
            using (var vwDynamic = new VowpalWabbitDynamic(new VowpalWabbitSettings("--cb_adf --rank_all") { TypeInspector = JsonTypeInspector.Default }))
            {
                var expected = vw.Learn(new[] { "| q:1", "2:-3:0.9 | q:2", "| q:3" }, VowpalWabbitPredictionType.Multilabel);
                var actual = vwDynamic.Learn(
                    new
                    {
                        _multi = new[]
                        {
                            new { q = 1 },
                            new { q = 2 },
                            new { q = 3 }
                        }
                    },
                    VowpalWabbitPredictionType.Multilabel,
                    new ContextualBanditLabel(2, -3, 0.9f),
                    2);

                CollectionAssert.AreEqual(expected, actual);

                expected = vw.Learn(new[] { "| q:1", "2:-5:0.9 | q:2", "| q:3" }, VowpalWabbitPredictionType.Multilabel);
                actual = vwDynamic.Learn(
                    new
                    {
                        _multi = new[]
                        {
                            new { q = 1 },
                            new { q = 2 },
                            new { q = 3 }
                        }
                    },
                    VowpalWabbitPredictionType.Multilabel,
                    new ContextualBanditLabel(2, -3, 0.9f),
                    2);

                CollectionAssert.AreEqual(expected, actual);

                using (var mem1 = new MemoryStream())
                using (var mem2 = new MemoryStream())
                {
                    vw.SaveModel(mem1);
                    vwDynamic.Native.SaveModel(mem2);

                    CollectionAssert.AreEqual(mem1.ToArray(), mem1.ToArray());
                }
            }
        }
    }
}
