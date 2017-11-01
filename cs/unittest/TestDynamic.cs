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
        [Ignore]
        [TestCategory("Vowpal Wabbit")]
        public void TestDynamic()
        {
            // TODO: look into friend assemblies and how to figure if one is a friend
            using (var vw = new VowpalWabbit("--cb_adf --rank_all"))
            using (var vwDynamic = new VowpalWabbitDynamic(new VowpalWabbitSettings("--cb_adf --rank_all") { TypeInspector = JsonTypeInspector.Default }))
            {
                var expected = vw.Learn(new[] { "| q:1", "2:-3:0.9 | q:2", "| q:3" }, VowpalWabbitPredictionType.ActionProbabilities);
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
                AssertAreEqual(expected, actual);

                expected = vw.Learn(new[] { "| q:1", "2:-5:0.9 | q:2", "| q:3" }, VowpalWabbitPredictionType.ActionProbabilities);
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
                AssertAreEqual(expected, actual);

                expected = vw.Learn(new[] { "| q:1", "| q:2", "3:-2:0.8 | q:3" }, VowpalWabbitPredictionType.ActionProbabilities);
                actual = vwDynamic.Learn(
                    new
                    {
                        _multi = new[]
                        {
                            new { q = 1 },
                            new { q = 2 },
                            new { q = 3 }
                        },
                        _labelIndex = 2,
                        _label_Action = 3,
                        _label_Cost = -2,
                        _label_Probability = 0.8
                    },
                    VowpalWabbitPredictionType.ActionScore);
                AssertAreEqual(expected, actual);
            }
        }

        private void AssertAreEqual(ActionScore[] expected, ActionScore[] actual)
        {
            Assert.AreEqual(expected.Length, actual.Length);
            for (int i = 0; i < expected.Length; i++)
            {
                Assert.AreEqual(expected[i].Action, actual[i].Action);
                Assert.AreEqual(expected[i].Score, actual[i].Score, 0.0001);
            }

            CollectionAssert.AreEqual(
                expected: Enumerable.Range(0, expected.Length).Select(i => (uint)i).ToList(),
                actual: actual.Select(a => a.Action).OrderBy(a => a).ToList());
        }
    }
}
