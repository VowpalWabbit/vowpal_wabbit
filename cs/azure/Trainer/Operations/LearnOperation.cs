using Microsoft.ApplicationInsights.DataContracts;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Reactive.Linq;
using VW;
using VW.Labels;

namespace VowpalWabbit.Azure.Trainer
{
    internal sealed class TrainerResult
    {
        public ActionScore[] ProgressivePrediction { get; set; }

        public ContextualBanditLabel Label { get; set; }

        public TimeSpan Latency { get; set; }

        public string PartitionKey { get; set; }
    }

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
                            { "VW", example.Example.VowpalWabbitString }
                        });

                var progressivePrediction = example.Example.Learn(VowpalWabbitPredictionType.ActionScore, this.vw);

                var label = example.Example.Labels
                    .OfType<ContextualBanditLabel>()
                    .FirstOrDefault(l => l.Probability != 0f || l.Cost != 0);
                
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

                // TODO: not sure which timestamp to use yet
                // var latency = DateTime.UtcNow - example.Timestamp;
                var latency = TimeSpan.FromSeconds(1);
                var performanceCounterTicks =
                    latency.Ticks * Stopwatch.Frequency / TimeSpanTicksPerMillisecond;
                this.perfCounters.AverageExampleLatency.IncrementBy(performanceCounterTicks);
                this.perfCounters.AverageExampleLatencyBase.Increment();

                // update partition state
                if (example.PartitionKey != null && example.PartitionKey != null)
                {
                    this.state.Partitions[example.PartitionKey] = example.Offset;
                    // this.state.PartitionsDateTime[eventHubExample.PartitionKey] = eventHubExample.Offset;
                }

                return new TrainerResult
                {
                    Label = label,
                    ProgressivePrediction = progressivePrediction,
                    PartitionKey = example.PartitionKey,
                    Latency = latency
                };
            }
            catch (Exception ex)
            {
                this.telemetry.TrackException(ex);
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
