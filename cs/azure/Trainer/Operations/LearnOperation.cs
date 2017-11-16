// --------------------------------------------------------------------------------------------------------------------
// <copyright file="Learner.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using Microsoft.ApplicationInsights.DataContracts;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Reactive.Linq;
using VW.Azure.Trainer.Data;
using VW.Labels;

namespace VW.Azure.Trainer
{
    internal partial class Learner
    {
        public TrainerResult Learn(PipelineData example)
        {
            try
            {
                if (this.settings.EnableExampleTracing)
                    this.telemetry.TrackTrace(
                        "Example",
                        SeverityLevel.Verbose,
                        new Dictionary<string, string>
                        {
                            { "ID", example.EventId },
                            { "VW", example.Example.VowpalWabbitString },
                            { "JSON", example.JSON }
                        });

                var label = example.Example.Labels
                    .OfType<ContextualBanditLabel>()
                    .FirstOrDefault(l => l.Probability != 0f || l.Cost != 0);

                if (label == null)
                    this.telemetry.TrackTrace($"Unable to find valid label for event '{example.EventId}'", SeverityLevel.Warning);

                // predict first then learn to avoid information leak
                var progressivePrediction = example.Example.Learn(VowpalWabbitPredictionType.ActionProbabilities, this.vw);

                //if (this.vwAllReduce != null)
                //{
                //    this.vwAllReduce.Post(vw =>
                //    {
                //        var actions = example.Example.Learn(VowpalWabbitPredictionType.Multilabel, vw);

                //        PerformanceCounters.Instance.ExamplesLearnedTotal.Increment();
                //        PerformanceCounters.Instance.ExamplesLearnedSec.Increment();
                //        PerformanceCounters.Instance.FeaturesLearnedSec.IncrementBy((long)example.Example.NumberOfFeatures);

                //        example.Example.Dispose();
                //    });
                //}

                // record event id for reproducibility
                this.trackbackList.Add(example.EventId);

                this.perfCounters.Stage2_Learn_Total.Increment();
                this.perfCounters.Stage2_Learn_ExamplesPerSec.Increment();
                this.perfCounters.Stage2_Learn_FeaturesPerSec.IncrementBy((long)example.Example.NumberOfFeatures);

                // measure latency
                const int TimeSpanTicksPerMillisecond = 10000;

                var latency = DateTime.UtcNow - example.Timestamp;
                var performanceCounterTicks =
                    latency.Ticks * Stopwatch.Frequency / TimeSpanTicksPerMillisecond;
                this.perfCounters.AverageExampleLatency.IncrementBy(performanceCounterTicks);
                this.perfCounters.AverageExampleLatencyBase.Increment();

                // update partition state
                if (example.PartitionId != null && example.PartitionId != null)
                {
                    this.state.Partitions[example.PartitionId] = example.Offset;
                    // this.state.PartitionsDateTime[eventHubExample.PartitionKey] = eventHubExample.Offset;
                }

                return new TrainerResult(progressivePrediction, example.Actions, example.Probabilities)
                {
                    Label = label,
                    PartitionId = example.PartitionId,
                    PartitionKey = example.PartitionKey,
                    Latency = latency,
                    ProbabilityOfDrop = example.ProbabilityOfDrop,
                    ActionsTags = example.ActionsTags,
                    EventId = example.EventId,
                    Timestamp = example.Timestamp
                };
            }
            catch (Exception ex)
            {
                this.telemetry.TrackException(ex, 
                    new Dictionary<string, string>
                        {
                            { "ID", example.EventId },
                            { "VW", example.Example.VowpalWabbitString },
                            { "JSON", example.JSON }
                        });

                this.perfCounters.Stage2_Faulty_ExamplesPerSec.Increment();
                this.perfCounters.Stage2_Faulty_Examples_Total.Increment();
                return null;
            }
            finally
            {
                if (example.Example != null)
                    example.Example.Dispose();
            }
        }
    }
}
