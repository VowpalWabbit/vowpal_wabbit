//using Microsoft.Research.MultiWorldTesting.ExploreLibrary;
using Newtonsoft.Json;
using System;
using System.IO;
using System.Linq;
using System.Text;
using VW;
using VW.Labels;

namespace simulator
{
    public static class VowpalWabbitSimulator
    {
        public class SimulatorExample
        {
            private readonly int length;

            private readonly byte[] exampleBuffer;

            public float[] PDF { get; }

            public SimulatorExample(int numActions, int sharedContext)
            {
                // generate distinct per user context with 2 seperate prefered actions
                this.PDF = Enumerable.Range(0, numActions).Select(_ => 0.005f).ToArray();
                this.PDF[sharedContext] = 0.03f;

                this.exampleBuffer = new byte[32 * 1024];

                var str = JsonConvert.SerializeObject(
                    new
                    {
                        Version = "1",
                        EventId = "1", // can be ignored
                        a = Enumerable.Range(1, numActions).ToArray(),
                        c = new
                        {
                            // shared user context
                            U = new { C = sharedContext.ToString() },
                            _multi = Enumerable
                                .Range(0, numActions)
                                .Select(i => new { A = new { Constant = 1, Id = i.ToString() }, B = new { Id = i.ToString() } })
                                .ToArray()
                        },
                        p = Enumerable.Range(0, numActions).Select(i => 0.0f).ToArray()
                    });

                Console.WriteLine(str);

                // allow for \0 at the end
                this.length = Encoding.UTF8.GetBytes(str, 0, str.Length, exampleBuffer, 0);
                exampleBuffer[this.length] = 0;
                this.length++;
            }

            public VowpalWabbitMultiLineExampleCollection CreateExample(VowpalWabbit vw)
            {
                VowpalWabbitDecisionServiceInteractionHeader header;
                var examples = vw.ParseDecisionServiceJson(this.exampleBuffer, 0, this.length, true, out header);

                var adf = new VowpalWabbitExample[examples.Count - 1];
                examples.CopyTo(1, adf, 0, examples.Count - 1);

                return new VowpalWabbitMultiLineExampleCollection(vw, examples[0], adf);
            }
        }

        private static void ExportScoringModel(VowpalWabbit learner, ref VowpalWabbit scorer)
        {
            scorer?.Dispose();
            using (var memStream = new MemoryStream())
            {
                learner.SaveModel(memStream);

                memStream.Seek(0, SeekOrigin.Begin);

                // Note: the learner doesn't use save-resume as done online
                scorer = new VowpalWabbit(new VowpalWabbitSettings { Arguments = "--quiet", ModelStream = memStream });
            }
        }

        public static void Run(string ml_args, int initial_random, int tot_iter, int mod_iter, int rewardSeed, ulong vwSeed, int exp_iter, int numContexts, int numActions, string ml_args_snips)
        {
            // byte buffer outside so one can change the example and keep the memory around
            var exampleBuffer = new byte[32 * 1024];

            var randGen = new Random(rewardSeed);
            var userGen = new Random();

            var simExamples = Enumerable.Range(0, numContexts)
                .Select(i => new SimulatorExample(numActions, i))
                .ToArray();

            var scorerPdf = new float[numActions];
            var histPred = new int[numActions, numContexts];
            var histPred2 = new int[numActions, numContexts];
            var histActions = new int[numActions, numContexts];
            var histCost = new int[numActions, numContexts];
            var histContext = new int[numContexts];
            int clicks = 0;
            double snips_num = 0, snips_den = 0;

            using (var learner = new VowpalWabbit(ml_args))
            using (var learner2 = new VowpalWabbit(ml_args_snips))
            {
                VowpalWabbit scorer = null;

                scorer = new VowpalWabbit("--cb_explore_adf --epsilon 1 --quiet");
                for (int i = 1; i <= tot_iter; i++)
                {
                    // sample uniform among users
                    int userIndex = userGen.Next(simExamples.Length);
                    var simExample = simExamples[userIndex];
                    var pdf = simExample.PDF;

                    histContext[userIndex]++;

                    using (var ex = simExample.CreateExample(learner))
                    {
                        var scores = ex.Predict(VowpalWabbitPredictionType.ActionProbabilities, scorer);

                        var total = 0.0;

                        foreach (var actionScore in scores)
                        {
                            total += actionScore.Score;
                            scorerPdf[actionScore.Action] = actionScore.Score;
                        }

                        var draw = randGen.NextDouble() * total;
                        var sum = 0.0;
                        uint topAction = 0;
                        foreach (var actionScore in scores)
                        {
                            sum += actionScore.Score;
                            if(sum > draw)
                            {
                                topAction = actionScore.Action;
                                break;
                            }
                        }

                        int modelAction = (int)scores[0].Action;
                        if (i > initial_random)
                            histPred[modelAction, userIndex] += 1;
                        histActions[topAction, userIndex] += 1;

                        // simulate behavior
                        float cost = 0;
                        if (randGen.NextDouble() < pdf[topAction])
                        {
                            cost = -1;
                            histCost[topAction, userIndex] += 1;
                            clicks += 1;
                        }

                        ex.Examples[topAction].Label = new ContextualBanditLabel((uint)topAction, cost, scorerPdf[topAction]);

                        // simulate delay
                        if (i >= initial_random && (i % exp_iter == 0))
                        {
                            ExportScoringModel(learner, ref scorer);
                        }

                        // invoke learning
                        var oneStepAheadScores = ex.Learn(VowpalWabbitPredictionType.ActionProbabilities, learner);
                        histPred2[oneStepAheadScores[0].Action, userIndex] += 1;

                        var oneStepAheadScores2 = ex.Learn(VowpalWabbitPredictionType.ActionProbabilities, learner2);

                        // SNIPS
                        snips_num -= oneStepAheadScores2.First(f => f.Action == topAction).Score * cost / scorerPdf[topAction];
                        snips_den += oneStepAheadScores2.First(f => f.Action == topAction).Score / scorerPdf[topAction];

                        if (i % mod_iter == 0 || i == tot_iter)
                        {
                            Console.WriteLine(JsonConvert.SerializeObject(new
                            {
                                Iter = i,
                                clicks,
                                CTR = clicks / (float)i,
                                aveLoss = learner.PerformanceStatistics.AverageLoss,
                                CTR_snips = snips_num / snips_den,
                                CTR_ips = snips_num / (float)i,
                                aveLoss2 = learner2.PerformanceStatistics.AverageLoss,
                                snips_num,
                                snips_den,
                                histActions,
                                histPred,
                                histCost,
                                histContext,
                                pdf
                            }));
                        }
                    }
                }
                Console.WriteLine("---------------------");
                scorer?.Dispose();
            }
        }
    }
}
