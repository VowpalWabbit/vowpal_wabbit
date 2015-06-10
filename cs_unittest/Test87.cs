using Microsoft.Research.MachineLearning;
using Microsoft.Research.MachineLearning.Interfaces;
using Microsoft.Research.MachineLearning.Labels;
using Microsoft.Research.MachineLearning.Serializer.Attributes;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace cs_unittest
{
    [TestClass]
    public class Test87Class
    {
        [TestMethod]
        public void Test87()
        {
            using (var vw = new VowpalWabbit<Data, DataADF>("--cb_adf --rank_all"))
            {
                //shared | s_1 s_2
                //0:1.0:0.5 | a_1 b_1 c_1
                //| a_2 b_2 c_2
                //| a_3 b_3 c_3

                //| b_1 c_1 d_1
                //0:0.0:0.5 | b_2 c_2 d_2

                //| a_1 b_1 c_1 
                //| a_3 b_3 c_3

                var example = new Data
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

                var result = vw.Learn(example);

                Assert.ReferenceEquals(example.ActionDependentFeatures[0], result[0]);
                Assert.ReferenceEquals(example.ActionDependentFeatures[1], result[1]);
                Assert.ReferenceEquals(example.ActionDependentFeatures[2], result[2]);

                example = new Data
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

                result = vw.Learn(example);
                Assert.ReferenceEquals(example.ActionDependentFeatures[0], result[1]);
                Assert.ReferenceEquals(example.ActionDependentFeatures[1], result[0]);

                example = new Data
                {
                    ActionDependentFeatures = new[] {
                        new DataADF { Features = new [] { "a_1","b_1","c_1" } },
                        new DataADF { Features = new [] { "a_3","b_3","c_3" } }
                    }
                };
                result = vw.Predict(example);

                Assert.ReferenceEquals(example.ActionDependentFeatures[0], result[1]);
                Assert.ReferenceEquals(example.ActionDependentFeatures[1], result[0]);
            }
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
