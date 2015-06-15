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
            using (var vw = new VowpalWabbit<Data, DataADF>("--cb_adf --rank_all"))
            {
                Data[] sampleData = CreateSampleCbAdfData();

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
            
            Data[] sampleData = CreateSampleCbAdfData();

            using (var vw = new VowpalWabbit<Data, DataADF>("--cb_adf --rank_all"))
            {
                foreach (Data example in sampleData)
                {
                    vw.Learn(example);
                }
                vw.SaveModel(cbadfModelFile);
            }
            
            // Get ground truth predictions
            var expectedPredictions = new List<DataADF[]>();
            using (var vw = new VowpalWabbit<Data, DataADF>(string.Format("-t -i {0}", cbadfModelFile)))
            {
                foreach (Data example in sampleData)
                {
                    expectedPredictions.Add(vw.Predict(example));
                }
            }

            // Test synchronous VW instances using shared model
            using (var vwModel = new VowpalWabbitModel("-t", File.OpenRead(cbadfModelFile)))
            using (var vwShared1 = new VowpalWabbit<Data, DataADF>(vwModel))
            using (var vwShared2 = new VowpalWabbit<Data, DataADF>(vwModel))
            {
                for (int i = 0; i < sampleData.Length; i++)
                {
                    DataADF[] actualPrediction = vwShared1.Predict(sampleData[i]);
                    Assert.ReferenceEquals(expectedPredictions[i], actualPrediction);
                }
            }

            // Test concurrent VW instances using shared model and model pool
            using (var vwModel = new VowpalWabbitModel("-t", File.OpenRead(cbadfModelFile)))
            using (var vwPool = new ObjectPool<VowpalWabbit<Data, DataADF>>(new VowpalWabbitFactory<Data, DataADF>(vwModel)))
            {
                Parallel.For
                (
                    fromInclusive: 0,
                    toExclusive: 20,
                    parallelOptions: new ParallelOptions { MaxDegreeOfParallelism = Environment.ProcessorCount * 2 },
                    body: i =>
                    {
                        using (PooledObject<VowpalWabbit<Data, DataADF>> vwObject = vwPool.Get())
                        {
                            var actualPredictions = new List<DataADF[]>();
                            foreach (Data example in sampleData)
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

        private Data[] CreateSampleCbAdfData()
        {
            var sampleData = new Data[3];

            //shared | s_1 s_2
            //0:1.0:0.5 | a_1 b_1 c_1
            //| a_2 b_2 c_2
            //| a_3 b_3 c_3

            //| b_1 c_1 d_1
            //0:0.0:0.5 | b_2 c_2 d_2

            //| a_1 b_1 c_1 
            //| a_3 b_3 c_3

            sampleData[0] = new Data
            {
                Shared = new[] { "s_1", "s_2" },
                ActionDependentFeatures = new[] {
                        new DataADF
                        {
                            Features = new[] { "a_1", "b_1", "c_1" },
                            Label = new ContextualBanditLabel
                            {
                                Cost = 1f,
                                Probability = .5f
                            }
                        },
                        new DataADF { Features = new [] { "a_2","b_2","c_2" } },
                        new DataADF { Features = new [] { "a_3","b_3","c_3" } },
                    }
            };

            sampleData[1] = new Data
            {
                ActionDependentFeatures = new[] {
                        new DataADF { Features = new [] { "b_1","c_1","d_1" } },
                        new DataADF 
                        { 
                            Features = new [] { "b_2", "c_2", "d_2" },
                            Label = new ContextualBanditLabel
                            {
                                Cost = 0f,
                                Probability = .5f
                            }
                        },
                    }
            };

            sampleData[2] = new Data
            {
                ActionDependentFeatures = new[] {
                        new DataADF { Features = new [] { "a_1","b_1","c_1" } },
                        new DataADF { Features = new [] { "a_3","b_3","c_3" } }
                    }
            };

            return sampleData;
        }

        public class Data : SharedExample, IActionDependentFeatureExample<DataADF>
        {
            [Feature]
            public string[] Shared { get; set; }

            public IReadOnlyList<DataADF> ActionDependentFeatures { get; set; }
        }

        public class DataADF : IExample
        {
            [Feature]
            public string[] Features { get; set; }

            public override string ToString()
            {
                return string.Join(" ", this.Features);
            }

            public ILabel Label { get; set; }
        }
    }
}
