using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using VW;

namespace cs_unittest
{
    [TestClass]
    public class TestMultiClassPredictionFactory
    {

        [TestMethod]
        [TestCategory("Vowpal Wabbit")]
        public void TestMultiClassProbabilitiesFactory()
        {
            using (var vw = SetupVW())
            {
                var res = vw.Predict(" | a", VowpalWabbitPredictionType.MultiClassProbabilities);

                Assert.AreEqual(res.Count, 3);
                Assert.AreEqual(res.Values.Sum(), 1.0f);

                Assert.IsTrue(res[1] > res[3]);
                Assert.IsTrue(res[2] > res[3]);

                var res2 = vw.Predict(" | e", VowpalWabbitPredictionType.MultiClassProbabilities);
                Assert.IsTrue(res2[3] > res2[1]);
                Assert.IsTrue(res2[3] > res2[2]);
            }
        }

        private VowpalWabbit SetupVW()
        {
            var vw = new VowpalWabbit(" --probabilities --loss_function=logistic --oaa 3");

            vw.Learn("1 | a b");
            vw.Learn("2 | a c");
            vw.Learn("3 | c b e");

            return vw;
        }
    }


    
}
