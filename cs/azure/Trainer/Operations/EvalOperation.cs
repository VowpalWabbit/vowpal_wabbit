// --------------------------------------------------------------------------------------------------------------------
// <copyright file="EvalData.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using Microsoft.ApplicationInsights;
using Microsoft.ServiceBus.Messaging;
using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Reactive.Linq;
using System.Text;
using System.Threading.Tasks.Dataflow;
using VW.Azure.Trainer.Data;

namespace VW.Azure.Trainer.Operations
{
    internal sealed class EvalData
    {
        internal string JSON { get; set; }

        internal string PolicyName { get; set; }
    }

    internal sealed class EvalOperation : IDisposable
    {
        private readonly EventHubClient evalEventHubClient;

        private TransformManyBlock<object, EvalData> evalBlock;
        private IDisposable evalBlockDisposable;
        private TelemetryClient telemetry;
        private PerformanceCounters performanceCounters;

        internal EvalOperation(OnlineTrainerSettingsInternal settings, PerformanceCounters performanceCounters) 
        {
            this.performanceCounters = performanceCounters;

            this.telemetry = new TelemetryClient();

            // evaluation pipeline 
            this.evalEventHubClient = EventHubClient.CreateFromConnectionString(settings.EvalEventHubConnectionString);

            this.evalBlock = new TransformManyBlock<object, EvalData>(
                (Func<object, IEnumerable<EvalData>>)this.OfflineEvaluate,
                new ExecutionDataflowBlockOptions
                {
                    MaxDegreeOfParallelism = 4,
                    BoundedCapacity = 1024
                });

            this.evalBlock.Completion.Trace(this.telemetry, "Stage 4 - Evaluation pipeline");

            // batch output together to match EventHub throughput by maintaining maximum latency of 1 seconds
            this.evalBlockDisposable = this.evalBlock.AsObservable()
                .GroupBy(k => k.PolicyName)
                   .Select(g =>
                        g.Window(TimeSpan.FromSeconds(1))
                         .Select(w => w.Buffer(245 * 1024, e => Encoding.UTF8.GetByteCount(e.JSON)))
                         .SelectMany(w => w)
                         .Subscribe(this.UploadEvaluation))
                   .Publish()
                   .Connect();
        }

        internal ITargetBlock<object> TargetBlock { get { return this.evalBlock; } }

        private List<EvalData> OfflineEvaluate(object trainerResult)
        {
            try
            {
                return this.OfflineEvaluateInternal(trainerResult as TrainerResult)
                    .ToList();
            }
            catch (Exception e)
            {
                this.telemetry.TrackException(e);

                return new List<EvalData>();
            }
        }

        private IEnumerable<EvalData> OfflineEvaluateInternal(TrainerResult trainerResult)
        {
            this.performanceCounters.Stage4_Evaluation_PerSec.Increment();
            this.performanceCounters.Stage4_Evaluation_Total.Increment();

            if (trainerResult == null)
            {
                this.telemetry.TrackTrace($"Received invalid data: trainerResult is null");
                yield break;
            }

            if (trainerResult.Label == null)
            {
                this.telemetry.TrackTrace($"Received invalid data: trainerResult.Label is null");
                yield break;
            }

            if (trainerResult.ProgressiveProbabilities == null)
            {
                this.telemetry.TrackTrace($"Received invalid data: trainerResult.Probabilities is null");
                yield break;
            }

            var pi_a_x = trainerResult.ProgressiveProbabilities[trainerResult.Label.Action - 1];
            var p_a_x = trainerResult.Label.Probability * (1 - trainerResult.ProbabilityOfDrop);

            yield return new EvalData
            {
                PolicyName = "Latest Policy",
                JSON = JsonConvert.SerializeObject(
                    new
                    {
                        name = "Latest Policy",
                        // calcuate expectation under current randomized policy (using current exploration strategy)
                        // VW action is 0-based, label Action is 1 based
                        cost = (trainerResult.Label.Cost * pi_a_x) / p_a_x,
                        prob = pi_a_x / p_a_x
                    })
            };

            // the one currently running
            yield return new EvalData
            {
                PolicyName = "Deployed Policy",
                JSON = JsonConvert.SerializeObject(
                    new
                    {
                        name = "Deployed Policy",
                        cost = trainerResult.Label.Cost,
                        prob = 1 // for deployed policy just use the observed cost
                    })
            };

            for (int action = 1; action <= trainerResult.ProgressiveRanking.Length; action++)
            {
                string tag;
                if (!trainerResult.ActionsTags.TryGetValue(action, out tag))
                    tag = action.ToString(CultureInfo.InvariantCulture);

                var name = $"Constant Policy {tag}";
                yield return new EvalData
                {
                    PolicyName = name,
                    JSON = JsonConvert.SerializeObject(
                    new
                    {
                        name = name,
                        cost = VowpalWabbitContextualBanditUtil.GetUnbiasedCost(trainerResult.Label.Action, (uint)action, trainerResult.Label.Cost, trainerResult.Label.Probability),
                        prob = trainerResult.Label.Action == action ? 1 / (trainerResult.ObservedProbabilities[action - 1] * (1 - trainerResult.ProbabilityOfDrop)) : 0
                    })
                };
            }
        }

        private void UploadEvaluation(IList<EvalData> batch)
        {
            try
            {
                this.performanceCounters.Stage4_Evaluation_Total.Increment();
                this.performanceCounters.Stage4_Evaluation_BatchesPerSec.Increment();

                var eventData = new EventData(Encoding.UTF8.GetBytes(string.Join("\n", batch.Select(b => b.JSON))))
                {
                    PartitionKey = batch.First().PolicyName
                };

                this.evalEventHubClient.Send(eventData);
            }
            catch (Exception e)
            {
                this.telemetry.TrackException(e);
            }
        }

        public void Dispose()
        {
            if (this.evalBlock != null)
            {
                this.evalBlock.Complete();
                this.evalBlock.Completion.Wait(TimeSpan.FromMinutes(1));
                this.evalBlock = null;
            }

            if (this.evalBlockDisposable != null)
            {
                this.evalBlockDisposable.Dispose();
                this.evalBlockDisposable = null;
            }
        }
    }
}
