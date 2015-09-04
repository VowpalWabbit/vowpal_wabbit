using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Threading.Tasks;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using VW;
using VW.Interfaces;
using VW.Labels;
using VW.Serializer.Attributes;

namespace cs_unittest
{
    [TestClass]
    public class TestCbAdfClass : TestBase
    {
        public void ProfilePerformanceWithStringData()
        {
            string outModelFile = "profile_cb_adf.model";
            using (var vw = new VowpalWabbit<DataString, DataStringADF>("--cb_adf --rank_all"))
            {
                DataString[] sampleData = CreateStringCbAdfData(1000 * 1000);
                foreach (DataString example in sampleData)
                {
                    vw.Learn(example);
                }
                vw.SaveModel(outModelFile);
            }
            File.Delete(outModelFile);
        }

        public void ProfilePerformanceWithFloatData()
        {
            string outModelFile = "profile_cb_adf.model";
            using (var vw = new VowpalWabbit<DataFloat, DataFloatADF>("--cb_adf --rank_all"))
            {
                DataFloat[] sampleData = CreateFloatCbAdfData(1000 * 1000);
                foreach (DataFloat example in sampleData)
                {
                    vw.Learn(example);
                }
                vw.SaveModel(outModelFile);
            }
            File.Delete(outModelFile);
        }

        [TestMethod]
        public void Test87()
        {
            using (var vw = new VowpalWabbit<DataString, DataStringADF>("--cb_adf --rank_all"))
            {
                var sampleData = CreateSampleCbAdfData();

                var example = sampleData[0];

                var result = vw.LearnAndPredict(example);

                ReferenceEquals(example.ActionDependentFeatures[0], result[0]);
                ReferenceEquals(example.ActionDependentFeatures[1], result[1]);
                ReferenceEquals(example.ActionDependentFeatures[2], result[2]);

                example = sampleData[1];

                result = vw.LearnAndPredict(example);
                ReferenceEquals(example.ActionDependentFeatures[0], result[1]);
                ReferenceEquals(example.ActionDependentFeatures[1], result[0]);

                example = sampleData[2];
                result = vw.Predict(example);

                ReferenceEquals(example.ActionDependentFeatures[0], result[1]);
                ReferenceEquals(example.ActionDependentFeatures[1], result[0]);
            }
        }

        [TestMethod]
        public void TestSharedModel()
        {
            string cbadfModelFile = "models/cb_adf.model";
            
            var sampleData = CreateSampleCbAdfData();

            using (var vw = new VowpalWabbit<DataString, DataStringADF>("--cb_adf --rank_all"))
            {
                foreach (DataString example in sampleData)
                {
                    vw.Learn(example);
                }
                vw.SaveModel(cbadfModelFile);
            }
            
            // Get ground truth predictions
            var expectedPredictions = new List<DataStringADF[]>();
            using (var vw = new VowpalWabbit<DataString, DataStringADF>(string.Format("-t -i {0}", cbadfModelFile)))
            {
                foreach (DataString example in sampleData)
                {
                    expectedPredictions.Add(vw.Predict(example));
                }
            }

            // Test synchronous VW instances using shared model
            using (var vwModel = new VowpalWabbitModel("-t", File.OpenRead(cbadfModelFile)))
            using (var vwShared1 = new VowpalWabbit<DataString, DataStringADF>(vwModel))
            using (var vwShared2 = new VowpalWabbit<DataString, DataStringADF>(vwModel))
            {
                for (int i = 0; i < sampleData.Length; i++)
                {
                    DataStringADF[] actualPrediction = vwShared1.Predict(sampleData[i]);
                    ReferenceEquals(expectedPredictions[i], actualPrediction);
                }
            }

            // Test concurrent VW instances using shared model and model pool
            using (var vwModel = new VowpalWabbitModel("-t", File.OpenRead(cbadfModelFile)))
            using (var vwPool = new ObjectPool<VowpalWabbit<DataString, DataStringADF>>(new VowpalWabbitFactory<DataString, DataStringADF>(vwModel)))
            {
                Parallel.For
                (
                    fromInclusive: 0,
                    toExclusive: 20,
                    parallelOptions: new ParallelOptions { MaxDegreeOfParallelism = Environment.ProcessorCount * 2 },
                    body: i =>
                    {
                        using (PooledObject<VowpalWabbit<DataString, DataStringADF>> vwObject = vwPool.Get())
                        {
                            var actualPredictions = new List<DataStringADF[]>();
                            foreach (DataString example in sampleData)
                            {
                                actualPredictions.Add(vwObject.Value.Predict(example));
                            }

                            Assert.AreEqual(expectedPredictions.Count, actualPredictions.Count);
                            for (int j = 0; j < expectedPredictions.Count; j++)
                            {
                                ReferenceEquals(expectedPredictions[j], actualPredictions[j]);
                            }
                        }
                    }
                );
            }
        }

        private DataString[] CreateSampleCbAdfData()
        {
            var sampleData = new DataString[3];

            //shared | s_1 s_2
            //0:1.0:0.5 | a_1 b_1 c_1
            //| a_2 b_2 c_2
            //| a_3 b_3 c_3

            //| b_1 c_1 d_1
            //0:0.0:0.5 | b_2 c_2 d_2

            //| a_1 b_1 c_1 
            //| a_3 b_3 c_3

            sampleData[0] = new DataString
            {
                Shared = new[] { "s_1", "s_2" },
                ActionDependentFeatures = new[] {
                        new DataStringADF
                        {
                            Features = new[] { "a_1", "b_1", "c_1" },
                            Label = new ContextualBanditLabel
                            {
                                Cost = 1f,
                                Probability = .5f
                            }
                        },
                        new DataStringADF { Features = new [] { "a_2","b_2","c_2" } },
                        new DataStringADF { Features = new [] { "a_3","b_3","c_3" } },
                    }
            };

            sampleData[1] = new DataString
            {
                ActionDependentFeatures = new[] {
                        new DataStringADF { Features = new [] { "b_1","c_1","d_1" } },
                        new DataStringADF 
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

            sampleData[2] = new DataString
            {
                ActionDependentFeatures = new[] {
                        new DataStringADF { Features = new [] { "a_1","b_1","c_1" } },
                        new DataStringADF { Features = new [] { "a_3","b_3","c_3" } }
                    }
            };

            return sampleData;
        }

        private DataString[] CreateStringCbAdfData(int numSamples, int randomSeed = 0)
        {
            var random = new Random(randomSeed);

            var sampleData = new DataString[numSamples];
            for (int i = 0; i < numSamples; i++)
            {
                int numActions = random.Next(2, 5);
                
                int[] fIndex = Enumerable.Range(1, numActions).OrderBy(ind => random.Next()).Take(numActions).ToArray();

                var features = new string[numActions][];
                for (int j = 0; j < numActions; j++)
                {
                    features[j] = new string[] 
                    { 
                        "a_" + fIndex[j],
                        "b_" + fIndex[j],
                        "c_" + fIndex[j],
                        "d_" + fIndex[j]
                    };
                }

                var adf = new DataStringADF[numActions];
                
                int labelIndex = random.Next(-1, numActions);
                
                for (int j = 0; j < numActions; j++)
                {
                    adf[j] = new DataStringADF { Features = features[j] };

                    if (j == labelIndex)
                    {
                        adf[j].Label = new ContextualBanditLabel
                        {
                            Cost = (float)random.NextDouble(),
                            Probability = (float)random.NextDouble()
                        };
                    }
                }

                sampleData[i] = new DataString
                {
                    ActionDependentFeatures = adf
                };
            }

            return sampleData;
        }

        private DataFloat[] CreateFloatCbAdfData(int numSamples, int randomSeed = 0)
        {
            var random = new Random(randomSeed);

            var sampleData = new DataFloat[numSamples];
            for (int i = 0; i < numSamples; i++)
            {
                int numActions = random.Next(2, 5);

                int[] fIndex = Enumerable.Range(1, numActions).OrderBy(ind => random.Next()).Take(numActions).ToArray();

                var features = new float[numActions][];
                for (int j = 0; j < numActions; j++)
                {
                    features[j] = new float[] 
                    { 
                        (fIndex[j] + 0) / (float)numActions,
                        (fIndex[j] + 1) / (float)numActions,
                        (fIndex[j] + 2) / (float)numActions,
                        (fIndex[j] + 3) / (float)numActions
                    };
                }

                var adf = new DataFloatADF[numActions];

                int labelIndex = random.Next(-1, numActions);

                for (int j = 0; j < numActions; j++)
                {
                    adf[j] = new DataFloatADF { Features = features[j] };

                    if (j == labelIndex)
                    {
                        adf[j].Label = new ContextualBanditLabel
                        {
                            Cost = (float)random.NextDouble(),
                            Probability = (float)random.NextDouble()
                        };
                    }
                }

                sampleData[i] = new DataFloat
                {
                    ActionDependentFeatures = adf
                };
            }

            return sampleData;
        }

        public class DataString : SharedExample, IActionDependentFeatureExample<DataStringADF>
        {
            [Feature]
            public string[] Shared { get; set; }

            public IReadOnlyList<DataStringADF> ActionDependentFeatures { get; set; }
        }

        public class DataFloat : SharedExample, IActionDependentFeatureExample<DataFloatADF>
        {
            [Feature]
            public string[] Shared { get; set; }

            public IReadOnlyList<DataFloatADF> ActionDependentFeatures { get; set; }
        }

        public class DataStringADF : IExample
        {
            [Feature]
            public string[] Features { get; set; }

            public override string ToString()
            {
                return string.Join(" ", this.Features);
            }

            public ILabel Label { get; set; }
        }

        public class DataFloatADF : IExample
        {
            [Feature]
            public float[] Features { get; set; }

            public override string ToString()
            {
                return string.Join(" ", this.Features);
            }

            public ILabel Label { get; set; }
        }
    }
}
