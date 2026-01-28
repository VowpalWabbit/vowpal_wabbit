using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Threading.Tasks;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using VW;
using VW.Labels;
using VW.Serializer.Attributes;
using Newtonsoft.Json;

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
                    vw.Learn(example, example.ActionDependentFeatures, example.SelectedActionIndex, example.Label);
                }
                vw.Native.SaveModel(outModelFile);
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
                    vw.Learn(example, example.ActionDependentFeatures, example.SelectedActionIndex, example.Label);
                }
                vw.Native.SaveModel(outModelFile);
            }
            File.Delete(outModelFile);
        }

        private void Validate(VowpalWabbitExampleValidator<DataString> vwSharedValidation, VowpalWabbitExampleValidator<DataStringADF> vwADFValidation, DataString example)
        {
            vwSharedValidation.Validate(example.Line, example, SharedLabel.Instance);
            for (int i = 0; i < example.ActionDependentFeatures.Count; i++)
            {
                var adf = example.ActionDependentFeatures[i];
                vwADFValidation.Validate(adf.Line, adf, i == example.SelectedActionIndex ? example.Label : null);
            }
        }

        public static void TestMemoryLeak()
        {
            string outModelFile = "cb_adf_mem_leak.model";
            using (var vw = new VowpalWabbit<DataString, DataStringADF>("--cb_adf --rank_all"))
            {
                DataString[] sampleData = CreateStringCbAdfData(1000);
                foreach (DataString example in sampleData)
                {
                    vw.Learn(example, example.ActionDependentFeatures, example.SelectedActionIndex, example.Label);
                }
                vw.Native.SaveModel(outModelFile);
            }

            var vwModel = new VowpalWabbitModel(new VowpalWabbitSettings(string.Format("--quiet -t -i {0}", outModelFile)) { MaxExampleCacheSize = 1024 });
            var pool = new VowpalWabbitThreadedPrediction<DataString, DataStringADF>(vwModel);

            while (true)
            {
                vwModel = new VowpalWabbitModel(new VowpalWabbitSettings(string.Format("--quiet -t -i {0}", outModelFile)) { MaxExampleCacheSize = 1024 });
                pool.UpdateModel(vwModel);
            }
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit/Command line through marshalling")]
        public void Test87()
        {
            using (var vw = new VowpalWabbit<DataString, DataStringADF>("--cb_adf --rank_all"))
            using (var vwSharedValidation = new VowpalWabbitExampleValidator<DataString>("--cb_adf --rank_all"))
            using (var vwADFValidation = new VowpalWabbitExampleValidator<DataStringADF>("--cb_adf --rank_all"))
            {
                var sampleData = CreateSampleCbAdfData();

                var example = sampleData[0];
                Validate(vwSharedValidation, vwADFValidation, example);

                var result = vw.LearnAndPredict(example, example.ActionDependentFeatures, example.SelectedActionIndex, example.Label);
                ReferenceEquals(example.ActionDependentFeatures[0], result[0]);
                ReferenceEquals(example.ActionDependentFeatures[1], result[1]);
                ReferenceEquals(example.ActionDependentFeatures[2], result[2]);

                example = sampleData[1];
                Validate(vwSharedValidation, vwADFValidation, example);

                result = vw.LearnAndPredict(example, example.ActionDependentFeatures, example.SelectedActionIndex, example.Label);
                ReferenceEquals(example.ActionDependentFeatures[0], result[1]);
                ReferenceEquals(example.ActionDependentFeatures[1], result[0]);

                example = sampleData[2];
                Validate(vwSharedValidation, vwADFValidation, example);

                result = vw.Predict(example, example.ActionDependentFeatures);

                ReferenceEquals(example.ActionDependentFeatures[0], result[1]);
                ReferenceEquals(example.ActionDependentFeatures[1], result[0]);
            }
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit")]
        public void TestSharedModel()
        {
#if NETCOREAPP3_0_OR_GREATER
            string cbadfModelFile = Path.Join("models", "cb_adf.model");
#else
            string cbadfModelFile = @"models\cb_adf.model";
#endif

            var sampleData = CreateSampleCbAdfData();

            using (var vw = new VowpalWabbit<DataString, DataStringADF>("--cb_adf --rank_all"))
            using (var vwSharedValidation = new VowpalWabbitExampleValidator<DataString>("--cb_adf --rank_all"))
            using (var vwADFValidation = new VowpalWabbitExampleValidator<DataStringADF>("--cb_adf --rank_all"))
            {
                foreach (DataString example in sampleData)
                {
                    Validate(vwSharedValidation, vwADFValidation, example);
                    vw.Learn(example, example.ActionDependentFeatures, example.SelectedActionIndex, example.Label);
                }
                vw.Native.SaveModel(cbadfModelFile);
            }

            // Get ground truth predictions
            var expectedPredictions = new List<DataStringADF[]>();
            using (var vw = new VowpalWabbit<DataString, DataStringADF>(string.Format("-t -i {0}", cbadfModelFile)))
            {
                foreach (DataString example in sampleData)
                {
                    var pred = vw.Predict(example, example.ActionDependentFeatures);

                    if (pred == null)
                        expectedPredictions.Add(null);
                    else
                    {
                        expectedPredictions.Add(pred.Select(p => p.Feature).ToArray());
                    }
                }
            }

            // Test synchronous VW instances using shared model
            using (var vwModel = new VowpalWabbitModel(new VowpalWabbitSettings("-t") { ModelStream = File.OpenRead(cbadfModelFile) }))
            using (var vwShared1 = new VowpalWabbit<DataString, DataStringADF>(new VowpalWabbitSettings{ Model = vwModel }))
            using (var vwShared2 = new VowpalWabbit<DataString, DataStringADF>(new VowpalWabbitSettings{ Model = vwModel }))
            {
                for (int i = 0; i < sampleData.Length; i++)
                {
                    var actualPrediction = vwShared1.Predict(sampleData[i], sampleData[i].ActionDependentFeatures);

                    if (actualPrediction == null)
                        ReferenceEquals(expectedPredictions[i], actualPrediction);
                    else
                        ReferenceEquals(expectedPredictions[i], actualPrediction.Select(p => p.Feature).ToArray());
                }
            }

            // Test concurrent VW instances using shared model and model pool
            using (var vwModel = new VowpalWabbitModel(new VowpalWabbitSettings("-t") { ModelStream = File.OpenRead(cbadfModelFile) }))
            using (var vwPool = new VowpalWabbitThreadedPrediction<DataString, DataStringADF>(vwModel))
            {
                Parallel.For
                (
                    fromInclusive: 0,
                    toExclusive: 20,
                    parallelOptions: new ParallelOptions { MaxDegreeOfParallelism = Environment.ProcessorCount * 2 },
                    body: i =>
                    {
                        using (var vwObject = vwPool.GetOrCreate())
                        {
                            var actualPredictions = new List<DataStringADF[]>();
                            foreach (DataString example in sampleData)
                            {
                                actualPredictions.Add(vwObject.Value.Predict(example, example.ActionDependentFeatures).Select(p => p.Feature).ToArray());
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

        [TestMethod]
        [TestCategory("Vowpal Wabbit")]
        public void TestCbAdfExplore()
        {
            var json = JsonConvert.SerializeObject(new
            {
                U = new { age = "18" },
                _multi = new[]
                {
                            new
                            {
                                G = new { _text = "this rocks" },
                                K = new { constant = 1, doc = "1" }
                            },
                            new
                            {
                                G = new { _text = "something NYC" },
                                K = new { constant = 1, doc = "2" }
                            },
                        },
                _label_Action = 2,
                _label_Probability = 0.1,
                _label_Cost = -1,
                _labelIndex = 1
            });

            using (var vw = new VowpalWabbitJson("--cb_explore_adf --bag 4 --epsilon 0.0001 --cb_type mtr --marginal K -q UG -b 24 --power_t 0 --l1 1e-9 -l 4e-3"))
            {
                for (int i = 0; i < 50; i++)
                {
                    var pred = vw.Learn(json, VowpalWabbitPredictionType.ActionProbabilities);
                    Assert.AreEqual(2, pred.Length);

                    if (i > 40)
                    {
                        Assert.AreEqual(1, (int)pred[0].Action);
                        Assert.IsTrue(pred[0].Score > .9);

                        Assert.AreEqual(0, (int)pred[1].Action);
                        Assert.IsTrue(pred[1].Score < .1);
                    }
                }

                vw.Native.SaveModel("cbadfexplore.model");
            }

            using (var vw = new VowpalWabbitJson(new VowpalWabbitSettings { Arguments = "-t", ModelStream = File.Open("cbadfexplore.model", FileMode.Open) }))
            {
                var predObj = vw.Predict(json, VowpalWabbitPredictionType.Dynamic);
                Assert.IsInstanceOfType(predObj, typeof(ActionScore[]));

                var pred = (ActionScore[])predObj;
                Assert.AreEqual(1, (int)pred[0].Action);
                Assert.IsTrue(pred[0].Score > .9);

                Assert.AreEqual(0, (int)pred[1].Action);
                Assert.IsTrue(pred[1].Score < .1);
            }

            using (var vwModel = new VowpalWabbitModel(new VowpalWabbitSettings { ModelStream = File.Open("cbadfexplore.model", FileMode.Open) }))
            using (var vwSeeded = new VowpalWabbitJson(new VowpalWabbitSettings { Model = vwModel }))
            {
                var pred = vwSeeded.Predict(json, VowpalWabbitPredictionType.ActionProbabilities);
                Assert.AreEqual(1, (int)pred[0].Action);
                Assert.IsTrue(pred[0].Score > .9);

                Assert.AreEqual(0, (int)pred[1].Action);
                Assert.IsTrue(pred[1].Score < .1);
            }

            using (var vwModel = new VowpalWabbitModel(new VowpalWabbitSettings { ModelStream = File.Open("cbadfexplore.model", FileMode.Open) }))
            {
                using (var vwPool = new VowpalWabbitJsonThreadedPrediction(vwModel))
                using (var vw = vwPool.GetOrCreate())
                {
                    var predObj = vw.Value.Predict(json, VowpalWabbitPredictionType.Dynamic);
                    Assert.IsInstanceOfType(predObj, typeof(ActionScore[]));

                    var pred = (ActionScore[])predObj;
                    Assert.AreEqual(1, (int)pred[0].Action);
                    Assert.IsTrue(pred[0].Score > .9);

                    Assert.AreEqual(0, (int)pred[1].Action);
                    Assert.IsTrue(pred[1].Score < .1);
                }
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
                Line = "shared | s_1 s_2",
                Shared = new[] { "s_1", "s_2" },
                ActionDependentFeatures = new[] {
                        new DataStringADF
                        {
                            Line = "0:1.0:0.5 | a_1 b_1 c_1",
                            Features = new[] { "a_1", "b_1", "c_1" },

                        },
                        new DataStringADF
                        {
                            Line = "| a_2 b_2 c_2",
                            Features = new [] { "a_2","b_2","c_2" }
                        },
                        new DataStringADF
                        {
                            Line = "| a_3 b_3 c_3",
                            Features = new [] { "a_3","b_3","c_3" }
                        },
                    },
                SelectedActionIndex = 0,
                Label = new ContextualBanditLabel
                {
                    Cost = 1f,
                    Probability = .5f
                }
            };

            sampleData[1] = new DataString
            {
                Line = string.Empty,
                ActionDependentFeatures = new[] {
                        new DataStringADF
                        {
                            Line = "| b_1 c_1 d_1",
                            Features = new [] { "b_1","c_1","d_1" }
                        },
                        new DataStringADF
                        {
                            Line = "0:0.0:0.5 | b_2 c_2 d_2",
                            Features = new [] { "b_2", "c_2", "d_2" }
                        },
                    },
                SelectedActionIndex = 1,
                Label = new ContextualBanditLabel
                {
                    Cost = 0f,
                    Probability = .5f
                }
            };

            sampleData[2] = new DataString
            {
                Line = string.Empty,
                ActionDependentFeatures = new[] {
                        new DataStringADF
                        {
                            Line = "| a_1 b_1 c_1 ",
                            Features = new [] { "a_1","b_1","c_1" }
                        },
                        new DataStringADF
                        {
                            Line = "| a_3 b_3 c_3",
                            Features = new [] { "a_3","b_3","c_3" }
                        }
                    }
            };

            return sampleData;
        }

        private static DataString[] CreateStringCbAdfData(int numSamples, int randomSeed = 0)
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

                for (int j = 0; j < numActions; j++)
                {
                    adf[j] = new DataStringADF { Features = features[j] };
                }

                sampleData[i] = new DataString
                {
                    ActionDependentFeatures = adf,
                    SelectedActionIndex = random.Next(-1, numActions),
                    Label = new ContextualBanditLabel
                    {
                        Cost = (float)random.NextDouble(),
                        Probability = (float)random.NextDouble()
                    }
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

                for (int j = 0; j < numActions; j++)
                {
                    adf[j] = new DataFloatADF { Features = features[j] };
                }

                sampleData[i] = new DataFloat
                {
                    ActionDependentFeatures = adf,
                    SelectedActionIndex = random.Next(-1, numActions),
                    Label = new ContextualBanditLabel
                    {
                        Cost = (float)random.NextDouble(),
                        Probability = (float)random.NextDouble()
                    }
                };
            }

            return sampleData;
        }

        public class DataString
        {
            public string Line { get; set; }

            [Feature]
            public string[] Shared { get; set; }

            public IReadOnlyList<DataStringADF> ActionDependentFeatures { get; set; }

            public int SelectedActionIndex { get; set; }

            public ILabel Label { get; set; }
        }

        public class DataFloat
        {
            [Feature]
            public string[] Shared { get; set; }

            public IReadOnlyList<DataFloatADF> ActionDependentFeatures { get; set; }

            public int SelectedActionIndex { get; set; }

            public ILabel Label { get; set; }
        }

        public class DataStringADF
        {
            public string Line { get; set; }

            [Feature]
            public string[] Features { get; set; }

            public override string ToString()
            {
                return string.Join(" ", this.Features);
            }
        }

        public class DataFloatADF
        {
            [Feature]
            public float[] Features { get; set; }

            public override string ToString()
            {
                return string.Join(" ", this.Features);
            }
        }
    }
}
