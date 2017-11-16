using System.Collections.Generic;
using VW;
using VW.Labels;
using VW.Serializer.Attributes;

namespace cs_unittest
{
    public static class TrainSetCs_testLdf
    {
        public static Cs_TestData[] CreateSampleCbAdfData()
        {
            var sampleData = new Cs_TestData[3];

            //shared | s_1 s_2
            //0:1.0:0.5 | a_1 b_1 c_1
            //| a_2 b_2 c_2
            //| a_3 b_3 c_3

            //| b_1 c_1 d_1
            //0:0.0:0.5 | b_2 c_2 d_2

            //| a_1 b_1 c_1
            //| a_3 b_3 c_3

            sampleData[0] = new Cs_TestData
            {
                Shared = new[] { "s_1", "s_2" },
                ActionDependentFeatures = new[] {
                        new Cs_TestCs_TestDataADF
                        {
                            Features = new[] { "a_1", "b_1", "c_1" },
                            Label = new ContextualBanditLabel
                            {
                                Cost = 1f,
                                Probability = .5f
                            }
                        },
                        new Cs_TestCs_TestDataADF { Features = new [] { "a_2","b_2","c_2" } },
                        new Cs_TestCs_TestDataADF { Features = new [] { "a_3","b_3","c_3" } },
                    }
            };

            sampleData[1] = new Cs_TestData
            {
                ActionDependentFeatures = new[] {
                        new Cs_TestCs_TestDataADF { Features = new [] { "b_1","c_1","d_1" } },
                        new Cs_TestCs_TestDataADF
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

            sampleData[2] = new Cs_TestData
            {
                ActionDependentFeatures = new[] {
                        new Cs_TestCs_TestDataADF { Features = new [] { "a_1","b_1","c_1" } },
                        new Cs_TestCs_TestDataADF { Features = new [] { "a_3","b_3","c_3" } }
                    }
            };

            return sampleData;
        }
    }

    public class Cs_TestData
    {
        [Feature]
        public string[] Shared { get; set; }

        public IReadOnlyList<Cs_TestCs_TestDataADF> ActionDependentFeatures { get; set; }
    }

    public class Cs_TestCs_TestDataADF
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
