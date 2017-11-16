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
        internal EvalEventData Data { get; set; }

        internal string JSON { get; set; }

        internal string PartitionKey { get; set; }
    }

    /// <summary>
    /// Evaluation Event Data
    /// </summary>
    public sealed class EvalEventData
    {
        /// <summary>
        /// Policy name
        /// </summary>
        [JsonProperty("name")]
        public string Name { get; set; }

        /// <summary>
        /// Weighted cost
        /// </summary>
        [JsonProperty("weightedcost")]
        public float WeightedCost { get; set; }

        /// <summary>
        /// Importance weight
        /// </summary>
        [JsonProperty("importanceweight")]
        public float ImportanceWeight { get; set; }

        /// <summary>
        /// Timestamp
        /// </summary>
        [JsonProperty("timestamp")]
        public DateTime Timestamp { get; set; }

        /// <summary>
        /// Event Id
        /// </summary>
        [JsonProperty("eventid")]
        public string EventId { get; set; }
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
                .GroupBy(k => k.PartitionKey)
                   .Select(g =>
                        g.Window(TimeSpan.FromSeconds(1))
                         .Select(w => w.Buffer(245 * 1024, e => Encoding.UTF8.GetByteCount(e.JSON)))
                         .SelectMany(w => w)
                         .Subscribe(this.UploadEvaluation))
                   .Publish()
                   .Connect();
        }

        internal ITargetBlock<object> TargetBlock { get { return this.evalBlock; } }

        private List<EvalData> OfflineEvaluate(object trainerResultObj)
        {
            try
            {
                var trainerResult = trainerResultObj as TrainerResult;
                if (trainerResult == null)
                {
                    this.telemetry.TrackTrace($"Received invalid data: trainerResult is null");
                    return new List<EvalData>();
                }

                return this.OfflineEvaluateInternal(trainerResult)
                    // insert event id & timestamp to enable data correlation
                    .Select(e => {
                        e.EventId = trainerResult.EventId;
                        e.Timestamp = trainerResult.Timestamp;

                        var ed = new EvalData
                        {
                            PartitionKey = trainerResult.PartitionKey,
                            Data = e,
                            JSON = JsonConvert.SerializeObject(e)
                        };

                        return ed;
                    })
                    .ToList();
            }
            catch (Exception e)
            {
                this.telemetry.TrackException(e);
                return new List<EvalData>();
            }
        }

        private IEnumerable<EvalEventData> OfflineEvaluateInternal(TrainerResult trainerResult)
        {
            this.performanceCounters.Stage4_Evaluation_PerSec.Increment();
            this.performanceCounters.Stage4_Evaluation_Total.Increment();

            if (trainerResult == null)
            {
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

            // the latest one we're currently training
            yield return new EvalEventData
            {
                Name = "Latest Policy",
                // calcuate expectation under current randomized policy (using current exploration strategy)
                // VW action is 0-based, label Action is 1 based
                WeightedCost = (trainerResult.Label.Cost * pi_a_x) / p_a_x,
                ImportanceWeight = pi_a_x / p_a_x
            };

            // the one currently running
            yield return new EvalEventData
            {
                Name = "Deployed Policy",
                WeightedCost = trainerResult.Label.Cost,
                ImportanceWeight = 1 // for deployed policy just use the observed cost
            };

            // Default = choosing the action that's supplied by caller
            yield return new EvalEventData
            {
                Name = "Default Policy",
                WeightedCost = VowpalWabbitContextualBanditUtil.GetUnbiasedCost(trainerResult.Label.Action, (uint)1, trainerResult.Label.Cost, trainerResult.Label.Probability),
                ImportanceWeight = trainerResult.Label.Action == 1 ? 1 / (trainerResult.ObservedProbabilities[0] * (1 - trainerResult.ProbabilityOfDrop)) : 0
            };

            // per action tag policies
            for (int action = 1; action <= trainerResult.ProgressiveRanking.Length; action++)
            {
                string tag;
                if (!trainerResult.ActionsTags.TryGetValue(action, out tag))
                    tag = action.ToString(CultureInfo.InvariantCulture);

                var name = $"Constant Policy {tag}";
                yield return new EvalEventData
                {
                    Name = name,
                    WeightedCost = VowpalWabbitContextualBanditUtil.GetUnbiasedCost(trainerResult.Label.Action, (uint)action, trainerResult.Label.Cost, trainerResult.Label.Probability),
                    ImportanceWeight = trainerResult.Label.Action == action ? 1 / (trainerResult.ObservedProbabilities[action - 1] * (1 - trainerResult.ProbabilityOfDrop)) : 0
                };
            }
        }

        private void UploadEvaluation(IList<EvalData> batch)
        {
            try
            {
                this.performanceCounters.Stage4_Evaluation_Total.Increment();
                this.performanceCounters.Stage4_Evaluation_BatchesPerSec.Increment();

                // construct multi-line JSON
                // TODO: check on how we batch in client library, we should use the EventId
                var eventData = new EventData(Encoding.UTF8.GetBytes(string.Join("\n", batch.Select(b => b.JSON))))
                {
                    PartitionKey = batch.First().PartitionKey
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
