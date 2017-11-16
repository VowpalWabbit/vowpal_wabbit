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
using System.Collections.Generic;
using System.Threading.Tasks;

namespace VW.Azure.Trainer
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
            this.telemetry.TrackTrace(
                $"OpenPartition Id {context.Lease.PartitionId}",
                SeverityLevel.Information,
                new Dictionary<string, string>
                {
                    { "PartitionId", context.Lease.PartitionId },
                    { "Offset", context.Lease.Offset }
                });

            this.perfCounters.EventHub_Processors.Increment();

            return Task.FromResult(true);
        }

        public async Task ProcessEventsAsync(PartitionContext context, IEnumerable<EventData> messages)
        {
            await this.parent.Stage0_Split(context, messages);
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
