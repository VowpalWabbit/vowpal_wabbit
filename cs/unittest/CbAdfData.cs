using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using VW.Labels;
using VW.Serializer.Attributes;

namespace cs_unittest.cbadf
{
    public class Generator
    {
        private static Random rand = new Random(123);

        public static Tuple<CbAdfShared, List<CbAdfAction>, ContextualBanditLabel> GenerateShared(int numActions)
        {
            return Tuple.Create(
                new CbAdfShared
                {
                    Num = rand.Next(100),
                    Vector = Enumerable.Range(1, 500).Select(_ => (float)rand.NextDouble()).ToArray()
                },
                Enumerable.Range(1, numActions).Select(_ => new CbAdfAction
                {
                    Vector = Enumerable.Range(1, 500).Select(__ => (float)rand.NextDouble()).ToArray()
                }).ToList(),
                new ContextualBanditLabel
                {
                    Action = (uint)rand.Next(numActions),
                    Cost = rand.Next(1),
                    Probability = (float)rand.NextDouble()
                });
        }
    }

    public class CbAdfShared
    {
        [Feature]
        public int Num { get; set; }

        [Feature(FeatureGroup = 'x', AddAnchor = true)]
        public float[] Vector { get; set; }
    }

    public class CbAdfAction
    {
        [Feature(FeatureGroup = 'y', AddAnchor = true)]
        public float[] Vector { get; set; }
    }
}
