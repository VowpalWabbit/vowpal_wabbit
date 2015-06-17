using Microsoft.Research.MachineLearning;
using Microsoft.Research.MachineLearning.Interfaces;
using Microsoft.Research.MachineLearning.Labels;
using Microsoft.Research.MachineLearning.Serializer.Attributes;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace cs_unittest
{
    [TestClass]
    public class TestCbAdfClass : TestBase
    {
        [TestMethod]
        public void Test87()
        {
            using (var vw = new VowpalWabbit<Cs_TestData, Cs_TestDataADF>("--cb_adf --rank_all"))
            {
                var sampleData = TrainSetCs_testLdf.CreateSampleCbAdfData();

                var example = sampleData[0];

                var result = vw.Learn(example);

                Assert.ReferenceEquals(example.ActionDependentFeatures[0], result[0]);
                Assert.ReferenceEquals(example.ActionDependentFeatures[1], result[1]);
                Assert.ReferenceEquals(example.ActionDependentFeatures[2], result[2]);

                example = sampleData[1];

                result = vw.Learn(example);
                Assert.ReferenceEquals(example.ActionDependentFeatures[0], result[1]);
                Assert.ReferenceEquals(example.ActionDependentFeatures[1], result[0]);

                example = sampleData[2];
                result = vw.Predict(example);

                Assert.ReferenceEquals(example.ActionDependentFeatures[0], result[1]);
                Assert.ReferenceEquals(example.ActionDependentFeatures[1], result[0]);
            }
        }

        [TestMethod]
        public void TestSharedModel()
        {
            string cbadfModelFile = "models/cb_adf.model";

            var sampleData = TrainSetCs_testLdf.CreateSampleCbAdfData();

            using (var vw = new VowpalWabbit<Cs_TestData, Cs_TestDataADF>("--cb_adf --rank_all"))
            {
                foreach (Cs_TestData example in sampleData)
                {
                    vw.Learn(example);
                }
                vw.SaveModel(cbadfModelFile);
            }
            
            // Get ground truth predictions
            var expectedPredictions = new List<Cs_TestDataADF[]>();
            using (var vw = new VowpalWabbit<Cs_TestData, Cs_TestDataADF>(string.Format("-t -i {0}", cbadfModelFile)))
            {
                foreach (Cs_TestData example in sampleData)
                {
                    expectedPredictions.Add(vw.Predict(example));
                }
            }

            // Test synchronous VW instances using shared model
            using (var vwModel = new VowpalWabbitModel("-t", File.OpenRead(cbadfModelFile)))
            using (var vwShared1 = new VowpalWabbit<Cs_TestData, Cs_TestDataADF>(vwModel))
            using (var vwShared2 = new VowpalWabbit<Cs_TestData, Cs_TestDataADF>(vwModel))
            {
                for (int i = 0; i < sampleData.Length; i++)
                {
                    Cs_TestDataADF[] actualPrediction = vwShared1.Predict(sampleData[i]);
                    Assert.ReferenceEquals(expectedPredictions[i], actualPrediction);
                }
            }

            // Test concurrent VW instances using shared model and model pool
            using (var vwModel = new VowpalWabbitModel("-t", File.OpenRead(cbadfModelFile)))
            using (var vwPool = new ObjectPool<VowpalWabbit<Cs_TestData, Cs_TestDataADF>>(new VowpalWabbitFactory<Cs_TestData, Cs_TestDataADF>(vwModel)))
            {
                Parallel.For
                (
                    fromInclusive: 0,
                    toExclusive: 20,
                    parallelOptions: new ParallelOptions { MaxDegreeOfParallelism = Environment.ProcessorCount * 2 },
                    body: i =>
                    {
                        using (PooledObject<VowpalWabbit<Cs_TestData, Cs_TestDataADF>> vwObject = vwPool.Get())
                        {
                            var actualPredictions = new List<Cs_TestDataADF[]>();
                            foreach (Cs_TestData example in sampleData)
                            {
                                actualPredictions.Add(vwObject.Value.Predict(example));
                            }

                            Assert.AreEqual(expectedPredictions.Count, actualPredictions.Count);
                            for (int j = 0; j < expectedPredictions.Count; j++)
                            {
                                Assert.ReferenceEquals(expectedPredictions[j], actualPredictions[j]);
                            }
                        }
                    }
                );
            }
        }
    }
}
