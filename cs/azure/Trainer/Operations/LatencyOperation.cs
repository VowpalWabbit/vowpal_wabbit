// --------------------------------------------------------------------------------------------------------------------
// <copyright file="LatencyOperation.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System;
using System.Threading.Tasks;
using VW.Azure.Trainer.Data;

namespace VW.Azure.Trainer
{
    internal sealed class LatencyOperation : ThrottledOperation<TrainerResult>
    {
        public LatencyOperation() : base(TimeSpan.FromSeconds(1))
        { }

        protected override Task ProcessInternal(TrainerResult value)
        {
            this.telemetry.TrackMetric("End-to-End Latency " + value.PartitionId, value.Latency.TotalSeconds);
            return Task.FromResult(true);
        }
    }
}
