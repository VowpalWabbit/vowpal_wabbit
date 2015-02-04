using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using MultiWorldTesting;

namespace cs_test
{
    public class ExploreClock
    {
        public static void Clock()
        {
            float epsilon = .2f;
            string uniqueKey = "clock";
            int numFeatures = 1000;
            int numIter = 1000;
            int numWarmup = 100;
            int numInteractions = 1;
            uint numActions = 10;

            double timeInit = 0, timeChoose = 0, timeSerializedLog = 0;

            System.Diagnostics.Stopwatch watch = new System.Diagnostics.Stopwatch();
            for (int iter = 0; iter < numIter + numWarmup; iter++)
            {
                watch.Restart();

                StringRecorder<SimpleContext> recorder = new StringRecorder<SimpleContext>();
                StringPolicy policy = new StringPolicy();
                MwtExplorer<SimpleContext> mwt = new MwtExplorer<SimpleContext>("mwt", recorder);
                EpsilonGreedyExplorer<SimpleContext> explorer = new EpsilonGreedyExplorer<SimpleContext>(policy, epsilon, numActions);

                timeInit += (iter < numWarmup) ? 0 : watch.Elapsed.TotalMilliseconds;

                Feature[] f = new Feature[numFeatures];
                for (int i = 0; i < numFeatures; i++)
                {
                    f[i].Id = (uint)i + 1;
                    f[i].Value = 0.5f;
                }

                watch.Restart();

                SimpleContext context = new SimpleContext(f);

                for (int i = 0; i < numInteractions; i++)
                {
                    mwt.ChooseAction(explorer, uniqueKey, context);
                }

                timeChoose += (iter < numWarmup) ? 0 : watch.Elapsed.TotalMilliseconds;

                watch.Restart();

                string interactions = recorder.GetRecording();

                timeSerializedLog += (iter < numWarmup) ? 0 : watch.Elapsed.TotalMilliseconds;

                for (int i = 0; i < numInteractions; i++)
                {
                    mwt.ChooseAction(explorer, uniqueKey, context);
                }
            }
            Console.WriteLine("--- PER ITERATION ---");
            Console.WriteLine("# iterations: {0}, # interactions: {1}, # context features {2}", numIter, numInteractions, numFeatures);
            Console.WriteLine("Init: {0} micro", timeInit * 1000 / numIter);
            Console.WriteLine("Choose Action: {0} micro", timeChoose * 1000 / (numIter * numInteractions));
            Console.WriteLine("Get Serialized Log: {0} micro", timeSerializedLog * 1000 / numIter);
            Console.WriteLine("--- TOTAL TIME: {0} micro", (timeInit + timeChoose + timeSerializedLog) * 1000);
        }
    }

    class StringPolicy : IPolicy<SimpleContext>
    {
        public uint ChooseAction(SimpleContext context)
        {
            return 1;
        }
    }
}
