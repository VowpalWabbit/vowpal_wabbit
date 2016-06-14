// --------------------------------------------------------------------------------------------------------------------
// <copyright file="LearnEventProcessor.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using Microsoft.ApplicationInsights;
using Microsoft.ApplicationInsights.DataContracts;
using Microsoft.ServiceBus.Messaging;
using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Reactive;
using System.Reactive.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Threading.Tasks.Dataflow;
using VowpalWabbit.Azure.Trainer.Data;
using VW;
using VW.Serializer;

namespace VowpalWabbit.Azure.Trainer
{
    internal sealed class LearnEventProcessor : IEventProcessor
    {
        private readonly TrainEventProcessorFactory parent;

        private readonly TelemetryClient telemetry;

        private readonly PerformanceCounters perfCounters;

        internal LearnEventProcessor(TrainEventProcessorFactory parent, PerformanceCounters perfCounters)
        {
            this.telemetry = new TelemetryClient();
            this.telemetry.Context.Component.Version = "TrainEventProcessor v" + GetType().Assembly.GetName().Version;

            this.parent = parent;
            this.perfCounters = perfCounters;
        }

        public Task OpenAsync(PartitionContext context)
        {
            this.telemetry.Context.Properties.Add("PartitionId", context.Lease.PartitionId);
            this.telemetry.Context.Properties.Add("Offset", context.Lease.Offset.ToString());

            this.telemetry.TrackTrace(
                $"OpenPartition Id {context.Lease.PartitionId}",
                SeverityLevel.Information,
                new Dictionary<string, string>
                {
                    { "PartitionId", context.Lease.PartitionId },
                    { "Offset", context.Lease.Offset.ToString() }
                });

            this.perfCounters.EventHub_Processors.Increment();

            return Task.FromResult(true);
        }

        public async Task ProcessEventsAsync(PartitionContext context, IEnumerable<EventData> messages)
        {
            foreach (EventData eventData in messages)
            {
                string eventMessage = string.Empty;

                using (var eventStream = eventData.GetBodyStream())
                {
                    using (var sr = new StreamReader(eventStream, Encoding.UTF8))
                    {
                        string line;
                        while ((line = await sr.ReadLineAsync()) != null)
                        {
                            var data = new PipelineData
                            {
                                JSON = line,
                                PartitionKey = context.Lease.PartitionId,
                                Offset = eventData.Offset
                            };

                            // TODO: ArrayBuffer to avoid string allocation...
                            // also just send char ref + offset + length
                            if (!await this.parent.DeserializeBlock.SendAsync(data))
                                this.telemetry.TrackTrace("Failed to enqueue data");
                        }

                        this.perfCounters.Stage0_IncomingBytesPerSec.IncrementBy(eventStream.Position);
                        this.perfCounters.Stage0_Batches_Size.IncrementBy(eventStream.Position);
                        this.perfCounters.Stage0_Batches_SizeBase.Increment();
                    }
                }

                this.perfCounters.Stage0_BatchesPerSec.Increment();
                this.perfCounters.Stage0_Batches_Total.Increment();
            }
        }

        public Task CloseAsync(PartitionContext context, CloseReason reason)
        {
            this.telemetry.TrackTrace(
                $"ClosePartition {context.Lease.PartitionId}: {reason}", 
                SeverityLevel.Information,
                new Dictionary<string, string>
                {
                    { "PartitionId", context.Lease.PartitionId },
                    { "Reason", reason.ToString() }
                });

            this.perfCounters.EventHub_Processors.Decrement();

            return Task.FromResult(true);
        }
    }
}
