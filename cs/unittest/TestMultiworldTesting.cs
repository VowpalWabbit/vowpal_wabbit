using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using VW;
using VW.Labels;

namespace cs_unittest
{
    [TestClass]
    public class TestMultiworldTestingClass
    {
        [TestMethod]
        [TestCategory("Vowpal Wabbit")]
        public void TestMultiworldTesting()
        {
            using (var mwt = new VowpalWabbitMultiworldTesting())
            {
                AssertAreEqual(
                    mwt.Evaluate(1, 2, new ContextualBanditLabel { Action = 1, Probability = .5f }),
                    0f, 0f, 0f);

                AssertAreEqual(
                    mwt.Evaluate(1, 2, new ContextualBanditLabel { Action = 2, Probability = .5f,  Cost = 1 }),
                    0f, 0f, 1f);

                AssertAreEqual(
                    mwt.Evaluate(1, 2, new ContextualBanditLabel { Action = 1, Probability = .5f }),
                    0, 0, 0.6666667f);
            }
        }

        private static void AssertAreEqual(VowpalWabbitMultiworldTesting.PoliciesPerformance actual, params float[] expected)
        {
            Assert.AreEqual(expected.Length, actual.NumConstantPolicies + 1);
            Assert.AreEqual(expected[0], actual.LearnedPolicy, 0.0001, "Learned policy differs");

            int i = 1;
            foreach (var value in actual.ConstantPolicies)
            {
                Assert.AreEqual(expected[i], value, 0.0001, "Constant policy " + i + " differs");
                i++;
            }
        }
    }
}
