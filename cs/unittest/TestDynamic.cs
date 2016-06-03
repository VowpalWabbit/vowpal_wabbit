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
                var expected = vw.Learn(new[] { "| q:1", "2:-3:0.9 | q:2", "| q:3" }, VowpalWabbitPredictionType.ActionScore);
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
                    VowpalWabbitPredictionType.ActionScore,
                    new ContextualBanditLabel(0, -3, 0.9f),
                    1);

                Assert.AreEqual(expected.Length, actual.Length);
                for (int i = 0; i < expected.Length; i++)
                {
                    Assert.AreEqual(expected[i].Action, actual[i].Action);
                    Assert.AreEqual(expected[i].Score, actual[i].Score, 0.0001);
                }

                expected = vw.Learn(new[] { "| q:1", "2:-5:0.9 | q:2", "| q:3" }, VowpalWabbitPredictionType.ActionScore);
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
                    VowpalWabbitPredictionType.ActionScore,
                    new ContextualBanditLabel(0, -5, 0.9f),
                    1);

                Assert.AreEqual(expected.Length, actual.Length);
                for (int i = 0; i < expected.Length; i++)
                {
                    Assert.AreEqual(expected[i].Action, actual[i].Action);
                    Assert.AreEqual(expected[i].Score, actual[i].Score, 0.0001);
                }
            }
        }
    }
}
