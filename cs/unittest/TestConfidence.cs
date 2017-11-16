using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using VW;
using VW.Labels;
using VW.Serializer.Attributes;

namespace cs_unittest
{
    [TestClass]
    public class TestConfidenceClass
    {
        public class Data
        {
            [Feature]
            public double Value { get; set; }
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit")]
        public void TestConfidence()
        {
            var rnd = new Random(42);
            using (var vw = new VowpalWabbit<Data>("--confidence -f model.conf --ngram 1 --bit_precision 8 --random_seed 123 --passes 2 -k -c model.conf.cache"))
            {
                for (int i = 0; i < 100; i++)
                {
                    if (i % 2 == 0)
                        vw.Learn(new Data { Value = rnd.NextDouble() }, new SimpleLabel { Label = 1 });
                    else
                        vw.Learn(new Data { Value = rnd.NextDouble() + 3 }, new SimpleLabel { Label = -1 });
                }

                vw.Native.RunMultiPass();

                var pred = vw.Predict(new Data { Value = 4.5 }, VowpalWabbitPredictionType.ScalarConfidence);
                Assert.AreEqual(-1f, pred.Value);
                Assert.IsTrue(pred.Confidence > 5);
            }

            Assert.IsTrue(File.Exists("model.conf.cache.cache"));
            File.Delete("model.conf.cache.cache");
            Assert.IsFalse(File.Exists("model.conf.cache.cache"));
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit")]
        public void TestConfidenceWithStringLines()
        {
            var rnd = new Random(42);
            using (var vw = new VowpalWabbit("--confidence -f model.conf --ngram 1 --bit_precision 18 --random_seed 123 --passes 2 -k -c model.conf.cache"))
            {
                for (int i = 0; i < 100; i++)
                {
                    if (i % 2 == 0)
                        vw.Learn(string.Format("{0} | :{1}", 1, rnd.NextDouble()));
                    else
                        vw.Learn(string.Format("{0} | :{1}", -1, rnd.NextDouble() + 3));
                }

                vw.Native.RunMultiPass();

                var pred = vw.Predict(string.Format(" | :{0}", 4.5), VowpalWabbitPredictionType.ScalarConfidence);
                Assert.AreEqual(-1f, pred.Value);
                Assert.IsTrue(pred.Confidence > 5);
            }
        }
    }
}
