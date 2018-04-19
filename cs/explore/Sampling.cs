using System;
using System.Collections;
using System.Linq;

namespace Microsoft.DecisionService.Exploration
{
    public class Sampling
    {
        public static int SampleAfterNormalizing(string seed, float[] probabilityDistribution)
        {
            bool pdfUpdate;
            return SampleAfterNormalizing(seed, probabilityDistribution, out pdfUpdate);
        }

        public static int SampleAfterNormalizing(string seed, float[] probabilityDistribution, out bool pdfUpdated)
        {
            ulong hash = MurMurHash3.ComputeIdHash(seed);
            float randomDraw = PRG.UniformUnitInterval(hash);
            return SampleAfterNormalizing(randomDraw, probabilityDistribution, out pdfUpdated);
        }

        public static int SampleAfterNormalizing(double draw, float[] probabilityDistribution)
        {
            bool pdfUpdate;
            return SampleAfterNormalizing(draw, probabilityDistribution, out pdfUpdate);
        }

        public static int SampleAfterNormalizing(double draw, float[] probabilityDistribution, out bool pdfUpdated)
        {
            pdfUpdated = false;

            // Create a discrete_distribution based on the returned weights. This class handles the
            // case where the sum of the weights is < or > 1, by normalizing agains the sum.
            float total = 0;
            for (int i = 0; i < probabilityDistribution.Length; i++)
            {
                if (probabilityDistribution[i] < 0)
                    probabilityDistribution[i] = 0;
                total += probabilityDistribution[i];
            }
            if (total == 0)
                throw new ArgumentOutOfRangeException("At least one score must be positive.");

            draw = total * draw;
            if (draw > total) //make very sure that draw can not be greater than total.
                draw = total;

            float action_probability = 0;
            int action_index = probabilityDistribution.Length - 1;
            float sum = 0;
            for (int i = 0; i < probabilityDistribution.Length; i++)
            {
                sum += probabilityDistribution[i];
                if (sum > draw)
                {
                    action_index = i;
                    action_probability = probabilityDistribution[i] / total;
                    break;
                }
            }

            return action_index;
        }

        public static int[] Sample(string seed, float[] probabilityDistribution, float[] scores)
        {
            bool pdfUpdated;
            return Sample(seed, probabilityDistribution, scores, out pdfUpdated);
        }

        /// <summary>
        /// Produce ranking
        /// </summary>
        public static int[] Sample(string seed, float[] probabilityDistribution, float[] scores, out bool pdfUpdated)
        {
            return SwapTopSlot(scores, SampleAfterNormalizing(seed, probabilityDistribution, out pdfUpdated));
        }

        private sealed class IndexComparer : IComparer
        {
            internal float[] scores;

            int IComparer.Compare(object x, object y)
            {
                return scores[(int)y].CompareTo(scores[(int)x]);
            }
        }

        public static int[] SwapTopSlot(float[] scores, int chosenAction)
        {
            int[] ranking = Enumerable.Range(0, scores.Length).ToArray();

            // use .NET Standard compatible sorting
            Array.Sort(ranking, new IndexComparer { scores = scores });

            SwapTopSlot(ranking, chosenAction);

            return ranking;
        }

        public static void SwapTopSlot(int[] ranking, int chosenAction)
        {
            int temp = ranking[0];
            ranking[0] = ranking[chosenAction];
            ranking[chosenAction] = temp;
        }
    }
}
